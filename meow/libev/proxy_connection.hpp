////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2017 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__PROXY_CONNECTION_HPP_
#define MEOW_LIBEV__PROXY_CONNECTION_HPP_

#include <functional>
#include <climits>

#include <meow/buffer.hpp>
#include <meow/defer.hpp>
#include <meow/error.hpp>
#include <meow/str_ref.hpp>
#include <meow/str_ref_algo.hpp>
#include <meow/format/format.hpp>
#include <meow/format/format_to_error.hpp>
#include <meow/convert/number_from_string.hpp>
#include <meow/libev/detail/generic_connection_impl.hpp>
#include <meow/unix/socket.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct proxy_connection_data_t
	{
		uint8_t                version;
		uint8_t                command;
		uint8_t                address_family;

		os_sockaddr_storage_t  src_addr;
		os_sockaddr_storage_t  dst_addr;
	};

	struct proxy_protocol_message_v2_t
	{
		struct {
			uint8_t  sig[12];    // magic signature
			uint8_t  ver_cmd;    // <version:4><command:4>
			uint8_t  fam_proto;  // <family:4 (AF_INET|AF_INET6|AF_UNIX)><transport_proto:4 (SOCK_STREAM|SOCK_DGRAM)>
			uint16_t len;        // remaining length, not including this header
		} hdr;

		union {
			struct {        /* for TCP/UDP over IPv4, len = 12 */
				uint32_t src_addr;
				uint32_t dst_addr;
				uint16_t src_port;
				uint16_t dst_port;
			} ipv4;

			struct {        /* for TCP/UDP over IPv6, len = 36 */
				uint8_t  src_addr[16];
				uint8_t  dst_addr[16];
				uint16_t src_port;
				uint16_t dst_port;
			} ipv6;

			struct {        /* for AF_UNIX sockets, len = 216 */
				uint8_t src_addr[108];
				uint8_t dst_addr[108];
			} unx;
		};
	};
	static_assert(sizeof(proxy_protocol_message_v2_t::hdr) == 16, "no padding expected");
	static_assert(sizeof(proxy_protocol_message_v2_t) == 232, "no padding expected");

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct proxy_connection_repack_traits : public Traits
	{
		using log_writer = typename Traits::log_writer; // needed for IO_LOG_WRITE to work, makes log_writer a dependent name
		using tr_read    = typename Traits::read;

		struct read
		{
			// http://www.haproxy.org/download/1.8/doc/proxy-protocol.txt states
			// 1. for v1 (text) a 108 byte buffer is always sufficient for proxy protocol header
			// 2. for v2 (binary) a 216 byte byffer is sufficient
			// so 256 bytes should be "enough for everybody" :)
			static constexpr size_t const network_buffer_size = 256;

			using callback_t = std::function<void(proxy_connection_data_t const&)>;

			// needed by generic_connection_impl_t
			struct context_t : public tr_read::context_t
			{
				callback_t       proxy_callback;
				buffer_move_ptr  proxy_rbuf;
				bool             proxy_enabled     = false;
				bool             proxy_got_headers = false;

				template<class Function>
				void proxy_set_data_callback(Function const& cb)
				{
					proxy_callback = cb;
				}

				void cb_proxy_data(proxy_connection_data_t const& pd) const
				{
					return this->proxy_callback(pd);
				}
			};


			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				if (!ctx->proxy_enabled) // not enabled, just pass to downstream
					return tr_read::get_buffer(ctx);

				if (ctx->proxy_got_headers) // headers read, just pass to downstream
				{
					assert(!ctx->proxy_rbuf && "proxy_connection_t proxy_rbuf must be empty after reading proxy protocol headers");
					return tr_read::get_buffer(ctx);
				}

				if (!ctx->proxy_rbuf)
					ctx->proxy_rbuf = meow::create_buffer(network_buffer_size);

				return ctx->proxy_rbuf->free_part();
			}

			template<class ContextT>
			static rd_consume_status_t consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
			{
				if (!ctx->proxy_enabled) // not enabled, just pass to downstream
					return tr_read::consume_buffer(ctx, read_part, r_status);

				if (ctx->proxy_got_headers) // just proxying
				{
					assert(!ctx->proxy_rbuf && "proxy_connection_t proxy_rbuf must be empty after reading proxy protocol headers");
					return tr_read::consume_buffer(ctx, read_part, r_status);
				}

				// now we might have our connection dead already
				if (read_status::error == r_status)
				{
					MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_closed, io_close_report(io_close_reason::io_error, errno));
					return rd_consume_status::closed;
				}

				// or closed
				if (read_status::closed == r_status)
				{
					MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_closed, io_close_report(io_close_reason::peer_close));
					return rd_consume_status::closed;
				}

				if (read_status::again == r_status && read_part.empty())
					return rd_consume_status::loop_break;

				// ok, looks like we've read something and connection is still alive

				buffer_move_ptr& b = ctx->proxy_rbuf;
				b->advance_last(read_part.size());

				// parse proxy header

				static str_ref const v1_sig    = meow::ref_lit("PROXY "); // note the space at end
				static size_t const  v1_maxlen = 107;

				static str_ref const v2_sig = meow::ref_lit("\x0D\x0A\x0D\x0A\x00\x0D\x0A\x51\x55\x49\x54\x0A");
				static size_t const  v2_maxlen = 232; // 216 for body + 16 for header

				// v2
				if (b->used_size() >= 16 && v2_sig == str_ref{b->first, v2_sig.size()})
				{
					auto const *msg = reinterpret_cast<proxy_protocol_message_v2_t const*>(b->first);

					size_t const size = 16 + ntohs(msg->hdr.len);
					if (size > b->used_size())
					{
						if (size > v2_maxlen) // we do not support v2 extensions as of yet
						{
							IO_LOG_WRITE(ctx, line_mode::single,
								"proxy_connection; got v2 header is too long {0} > max: {1}", size, v2_maxlen);
							return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
						}
						else
						{
							return rd_consume_status::more;
						}
					}

					// got full header read
					b->advance_first(size); // b now contains only upstream connection data

					proxy_connection_data_t pdata = {};
					auto const err = proxy_connection___parse_data_v2(&pdata, msg);
					if (err)
					{
						IO_LOG_WRITE(ctx, line_mode::single, "proxy_connection; v2 message error: {0}", err);
						return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
					}

					ctx->proxy_got_headers = true;
					ctx->cb_proxy_data(pdata);

					return proxy_connection___consume_rbuf_data(ctx, r_status);
				}
				// v1
				else if (b->used_size() >= 8 && v1_sig == str_ref{b->first, v1_sig.size()})
				{
					size_t const header_len = std::min(b->used_size(), v1_maxlen);

					char const *end = (char const*)memchr(b->first, '\r', header_len - 1);
					if (end == NULL) // no \r yet or not full header read
					{
						if (b->used_size() >= v1_maxlen) // header fully read, but no '\r' there, bail
						{
							IO_LOG_WRITE(ctx, line_mode::single,
								"proxy_connection; bad header, too long ({0} >= {1})", b->used_size(), v1_maxlen);

							return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
						}
						else
						{
							return rd_consume_status::more;
						}
					}

					if ('\n' != end[1]) // no LF after CR
					{
						IO_LOG_WRITE(ctx, line_mode::single, "proxy_connection; bad header, no LF found after CR");

						return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
					}

					str_ref const full_line = { b->first, end };
					str_ref const line      = { b->first + v1_sig.size(), end };
					b->reset_first((char*)end + 2); // buffer now contains data for upstream connection

					IO_LOG_WRITE(ctx, line_mode::single, "proxy_connection; got proxy line: '{0}'", full_line);

					proxy_connection_data_t pdata = {};
					auto const err = proxy_connection___parse_data_v1(&pdata, line);
					if (err)
					{
						IO_LOG_WRITE(ctx, line_mode::single, "proxy_connection___parse_data_v1: {0}", err);
						return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
					}

					ctx->proxy_got_headers = true;
					ctx->cb_proxy_data(pdata);

					return proxy_connection___consume_rbuf_data(ctx, r_status);
				}
				else // wrong protocol, signal error and close the connection
				{
					IO_LOG_WRITE(ctx, line_mode::single,
						"proxy_connection; bad proxy header data: {{ {0}, {1} }", b->used_size(), meow::format::as_hex_string(b->used_part()));

					return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
				}
			}

		private: // helpers

			static meow::error_t proxy_connection___parse_data_v2(proxy_connection_data_t *pd, proxy_protocol_message_v2_t const *msg)
			{
				uint8_t const version = (msg->hdr.ver_cmd & 0xf0) >> 4;
				if (version != 2)
					return meow::format::fmt_err("unexpected version: {0}", version);

				pd->version = version;

				uint8_t const command = (msg->hdr.ver_cmd & 0x0f);
				if (command == 0x00) // LOCAL command
				{
					pd->command        = command;
					pd->address_family = AF_UNSPEC;
					return {};
				}
				else if (command == 0x01) // PROXY command
				{
					pd->command = command;

					if (msg->hdr.fam_proto == 0x11) // (AF_INET + SOCK_STREAM) = TCP4
					{
						pd->address_family = AF_INET;
						pd->src_addr.ss_family = pd->address_family;
						pd->dst_addr.ss_family = pd->address_family;

						auto *src_sa = (os_sockaddr_in_t*)&pd->src_addr;
						auto *dst_sa = (os_sockaddr_in_t*)&pd->dst_addr;

						memcpy(&src_sa->sin_addr.s_addr, &msg->ipv4.src_addr, sizeof(src_sa->sin_addr.s_addr));
						memcpy(&dst_sa->sin_addr.s_addr, &msg->ipv4.dst_addr, sizeof(dst_sa->sin_addr.s_addr));
						src_sa->sin_port = msg->ipv4.src_port;
						dst_sa->sin_port = msg->ipv4.dst_port;
					}
					else if (msg->hdr.fam_proto == 0x21) // (AF_INET6 + SOCK_STREAM) = TCP6
					{
						pd->address_family = AF_INET6;
						pd->src_addr.ss_family = pd->address_family;
						pd->dst_addr.ss_family = pd->address_family;

						auto *src_sa = (os_sockaddr_in6_t*)&pd->src_addr;
						auto *dst_sa = (os_sockaddr_in6_t*)&pd->dst_addr;

						memcpy(&src_sa->sin6_addr, &msg->ipv6.src_addr, sizeof(src_sa->sin6_addr));
						memcpy(&dst_sa->sin6_addr, &msg->ipv6.dst_addr, sizeof(dst_sa->sin6_addr));
						src_sa->sin6_port = msg->ipv6.src_port;
						dst_sa->sin6_port = msg->ipv6.dst_port;
					}
					else
					{
						pd->address_family = AF_UNSPEC;
						// NOT parsing addresses, might be unix, but cba implementing that
					}

					return {};
				}
				else
				{
					return meow::format::fmt_err("unsupported command: {0}", meow::format::as_hex(command));
				}
			}

			static meow::error_t proxy_connection___parse_data_v1(proxy_connection_data_t *pd, str_ref line)
			{
				// parser helper
				auto const parse_port = [](uint32_t *out_port, str_ref port_s) -> meow::error_t
				{
					// FIXME: this depends on current locale

					bool const is_allowed_digit = (port_s[0] >= '1') && (port_s[0] <= '9');
					if (!is_allowed_digit)
						return meow::format::fmt_err("bad leading chars, need [1-9]");

					if (!meow::number_from_string(out_port, port_s))
						return meow::format::fmt_err("string is not a number");

					if (*out_port > USHRT_MAX)
						return meow::format::fmt_err("{0} >= 65535", *out_port);

					return {};
				};

				// XXX: simple split allows multiple consecutive spaces, which is against the spec, but whatever
				auto parts = meow::split_ex(line, " ");

				constexpr size_t const n_parts_expected = 5; // family + 2 addrs + 2 ports

				if (parts.size() == 1) // only one elt, must be a string "UNKNOWN"
					parts.resize(n_parts_expected); // missing ones will be empty

				if (parts.size() != n_parts_expected)
					return meow::format::fmt_err("bad number of 'words' in v1 header: {0}, need 1 or 5", parts.size());

				// ok, got correct number of parts, can parse now

				auto const af_s       = parts[0];
				auto const src_addr_s = parts[1];
				auto const dst_addr_s = parts[2];
				auto const src_port_s = parts[3];
				auto const dst_port_s = parts[4];

				pd->version = 1;
				pd->command = 0x1;

				if (af_s == meow::ref_lit("UNKNOWN"))
				{
					pd->address_family = AF_UNSPEC;
					return {};
				}
				else if (af_s == meow::ref_lit("TCP4"))
				{
					pd->address_family = AF_INET;
					pd->src_addr.ss_family = pd->address_family;
					pd->dst_addr.ss_family = pd->address_family;

					auto *src_sa = (os_sockaddr_in_t*)&pd->src_addr;
					auto *dst_sa = (os_sockaddr_in_t*)&pd->dst_addr;

					uint32_t src_port = 0;
					uint32_t dst_port = 0;

					if (1 != inet_pton(pd->address_family, src_addr_s.str().c_str(), (void*)&src_sa->sin_addr))
						return meow::format::fmt_err("error parsing src_addr '{0}'", src_addr_s);

					if (1 != inet_pton(pd->address_family, dst_addr_s.str().c_str(), (void*)&dst_sa->sin_addr))
						return meow::format::fmt_err("error parsing dst_addr '{0}'", dst_addr_s);

					if (auto const err = parse_port(&src_port, src_port_s))
						return meow::format::fmt_err("error parsing src_port '{0}': {1}", src_port_s, err);

					if (auto const err = parse_port(&dst_port, dst_port_s))
						return meow::format::fmt_err("error parsing dst_port '{0}': {1}", dst_port_s, err);

					src_sa->sin_port = htons(src_port);
					dst_sa->sin_port = htons(dst_port);

					return {};
				}
				else if (af_s == meow::ref_lit("TCP6"))
				{
					pd->address_family = AF_INET6;
					pd->src_addr.ss_family = pd->address_family;
					pd->dst_addr.ss_family = pd->address_family;

					auto *src_sa = (os_sockaddr_in6_t*)&pd->src_addr;
					auto *dst_sa = (os_sockaddr_in6_t*)&pd->dst_addr;

					uint32_t src_port = 0;
					uint32_t dst_port = 0;

					if (1 != inet_pton(pd->address_family, src_addr_s.str().c_str(), (void*)&src_sa->sin6_addr))
						return meow::format::fmt_err("error parsing src_addr '{0}'", src_addr_s);

					if (1 != inet_pton(pd->address_family, dst_addr_s.str().c_str(), (void*)&dst_sa->sin6_addr))
						return meow::format::fmt_err("error parsing dst_addr '{0}'", dst_addr_s);

					if (auto const err = parse_port(&src_port, src_port_s))
						return meow::format::fmt_err("error parsing src_port '{0}': {1}", src_port_s, err);

					if (auto const err = parse_port(&dst_port, dst_port_s))
						return meow::format::fmt_err("error parsing dst_port '{0}': {1}", dst_port_s, err);

					src_sa->sin6_port = htons(src_port);
					dst_sa->sin6_port = htons(dst_port);

					return {};
				}
				else
				{
					return meow::format::fmt_err("unknown v1 af: '{0}', need TCP4|TCP6|UNKNOWN", af_s);
				}
			}

			template<class ContextT>
			static rd_consume_status_t proxy_connection___consume_rbuf_data(ContextT *ctx, read_status_t r_status)
			{
				buffer_move_ptr& b = ctx->proxy_rbuf;

				MEOW_DEFER(
					IO_LOG_WRITE(ctx, line_mode::single,
						"proxy_connection___consume_rbuf_data; returning, buf: {0} {{ {1}, {2} }",
						b.get(), b ? b->used_size() : 0,
						meow::format::as_hex_string(b ? b->used_part() : buffer_ref{}));
				);

				// feed the rest of the buffer upstream!
				while (!b->empty())
				{
					IO_LOG_WRITE(ctx, line_mode::single,
						"proxy_connection___consume_rbuf_data; buf_data: {{ {0}, {1} }",
						__func__, b->used_size(), meow::format::as_hex_string(b->used_part()));

					buffer_ref data_buf = tr_read::get_buffer(ctx);
					if (!data_buf)
					{
						// try writing first and then poll for more
						// FIXME: taking this branch is basically guaranteed to not process all data
						//  if nothing more comes from the network
						// FIXME: actually, this branch is guaranteed to fail assertion in get_buffer()
						//  since we're not clearing proxy_rbuf here
						//  but returning something else is not an option either, since we'd loop infinitely in that case
						//  so just have an assert() right here for now
						assert(!"got loop_break from wrapped connection, can't handle it (see code comments)");
						return rd_consume_status::loop_break;
					}

					size_t const read_sz = std::min(data_buf.size(), b->used_size());

					IO_LOG_WRITE(ctx, line_mode::single,
						"proxy_connection___consume_rbuf_data; get_buffer() returned size: {0}, b->size(): {1}, using: {2}",
						__func__, data_buf.size(), b->used_size(), read_sz);

					memcpy(data_buf.begin(), b->first, read_sz);
					b->advance_first(read_sz);

					// since we're re-buffering data, must provide full or again
					read_status_t const rst = (read_sz == data_buf.size())
												? read_status::full
												: read_status::again;

					rd_consume_status_t rdc_status = tr_read::consume_buffer(ctx, buffer_ref(data_buf.data(), read_sz), rst);

					IO_LOG_WRITE(ctx, line_mode::single,
						"proxy_connection___consume_rbuf_data; tr_read::consume_buffer() -> {0}", __func__, rdc_status);

					switch (rdc_status) {
						case rd_consume_status::more:
							continue;

						case rd_consume_status::loop_break:
							// lie to wrapped connection and read more, before writing or other stuff
							// since there is no way to know that anything will ever come over the network again
							// and we need to feed the whole buffer to it (or we'd just lock up without processing data)
							continue;

						case rd_consume_status::closed:
							// returning 'closed', we're guaranteed to never be called again, so we should be fine
							return rdc_status;
					}
				}

				// clear buffer memory as soon as we're done
				b.reset();

				return rd_consume_status::more;
			}
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Interface
		, class Traits
		>
	struct proxy_connection_impl_t
		: public generic_connection_impl_t<Interface, proxy_connection_repack_traits<Traits> >
	{
		using connection_t      = proxy_connection_impl_t;
		using connection_traits = proxy_connection_repack_traits<Traits>;

		using impl_t   = generic_connection_impl_t<Interface, connection_traits>;
		using events_t = typename impl_t::events_t;

		using proxy_headers_callback_t = std::function<void(connection_t*, proxy_connection_data_t const&)>;

	public:

		proxy_connection_impl_t(evloop_t *loop, int fd, events_t *ev = NULL)
			: impl_t(loop, fd, ev)
		{
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__PROXY_CONNECTION_HPP_
