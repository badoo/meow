////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__SSL_CONNECTION_HPP_
#define MEOW_LIBEV_DETAIL__SSL_CONNECTION_HPP_

#include <sys/uio.h> // writev()

#include <stdexcept> // std::logic_error

#include <cyassl/ssl.h>
#include <cyassl/internal.h>

#include <meow/format/format.hpp>
#include <meow/format/format_tmp.hpp> // used for log_writer
#include <meow/format/format_to_string.hpp> // used for ssl_log_writer
#include <meow/format/inserter/hex_string.hpp>

#include "ssl.hpp"
#include "generic_connection.hpp"
#include "generic_connection_impl.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct ssl_connection_repack_traits : public Traits
	{
		struct ssl_write_traits__default
		{
			template<class ContextT> static bool is_allowed(ContextT *ctx) { return false; }
			template<class ContextT> static void write(ContextT *ctx, meow::line_mode_t, str_ref const&) {}
		};

		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, ssl_log_writer, ssl_write_traits__default);

		#define SSL_LOG_WRITE(ctx, lmode, fmt, ...) 				\
			do { if (ssl_log_writer::is_allowed(ctx))				\
				ssl_log_writer::write(ctx, lmode, meow::format::fmt_str(fmt, ##__VA_ARGS__));	\
			} while(0)												\
		/**/

		typedef typename Traits::log_writer log_writer;

		#define IO_LOG_WRITE(ctx, lmode, fmt, ...) 				\
			do { if (log_writer::is_allowed(ctx))				\
				log_writer::write(ctx, lmode, meow::format::fmt_tmp<1024>(fmt, ##__VA_ARGS__));	\
			} while(0)											\
		/**/

		typedef typename Traits::read tr_read;

		struct rw_context_t : public tr_read::context_t
		{
			buffer_move_ptr  ssl_rbuf;
			buffer_move_ptr  r_plaintext_buf;
			buffer_chain_t   w_plaintext_chain;
			ssl_move_ptr     rw_ssl;
		};

		template<class ContextT>
		static CYASSL* ssl_from_context(ContextT *ctx)
		{
			return ctx->rw_ssl.get();
		}

		template<class ContextT>
		static bool ctx_ssl_is_initialized(ContextT *ctx)
		{
			return !!ctx->rw_ssl;
		}

		struct read
		{
			typedef rw_context_t context_t;
			static size_t const network_buffer_size = 16 * 1024;

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				if (!ctx_ssl_is_initialized(ctx))
				{
					return tr_read::get_buffer(ctx);
				}
				else
				{
					buffer_move_ptr& b = ctx->ssl_rbuf;
					if (!b)
						b = create_buffer(network_buffer_size);

					return b->free_part();
				}
			}

			template<class ContextT>
			static rd_consume_status_t consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
			{
				if (!ctx_ssl_is_initialized(ctx))
				{
					return tr_read::consume_buffer(ctx, read_part, r_status);
				}

				if (read_status::error == r_status)
					return tr_read::consume_buffer(ctx, buffer_ref(), r_status);

				if (read_status::again == r_status && read_part.empty())
					return rd_consume_status::more;

				buffer_move_ptr& b = ctx->ssl_rbuf;
				b->advance_last(read_part.size());

				SSL_LOG_WRITE(ctx, line_mode::single, "{0}; {1} - {{ {2}, {3} }"
						, __func__, read_status::enum_as_str_ref(r_status)
						, read_part.size(), meow::format::as_hex_string(read_part));

				CYASSL *ssl = ssl_from_context(ctx);

				while (true)
				{
					buffer_ref data_buf = tr_read::get_buffer(ctx);

					if (!data_buf)
						return rd_consume_status::loop_break;

					int r = CyaSSL_read(ssl, data_buf.data(), data_buf.size());
					SSL_LOG_WRITE(ctx, line_mode::prefix, "SSL_read({0}, {1}, {2}) = {3}", ctx, (void*)data_buf.data(), data_buf.size(), r);

					if (r <= 0)
					{
						int err_code = CyaSSL_get_error(ssl, r);
						SSL_LOG_WRITE(ctx, line_mode::suffix, ", ssl_code: {0}", err_code);

						// connection ssl was shut down by processing a message
						if (SSL_ERROR_ZERO_RETURN == err_code)
							return tr_read::consume_buffer(ctx, buffer_ref(), read_status::closed);

						// not a real error, just nonblocking thingy
						if (SSL_ERROR_WANT_WRITE == err_code || SSL_ERROR_WANT_READ == err_code)
						{
							if (read_status::closed == r_status)
								return tr_read::consume_buffer(ctx, buffer_ref(), r_status);
							else
								return rd_consume_status::loop_break;
						}

						// real error
						int ssl_errno = ssl->error;
						char err_buf[1024] = {};
						CyaSSL_ERR_error_string_n(ssl_errno, err_buf, sizeof(err_buf));
						SSL_LOG_WRITE(ctx, line_mode::single, "SSL_read(): {0} - {1}", ssl_errno, err_buf);
						//TODO: ctx->ssl_cb_on_error(ssl_errno);

						return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
					}
					else
					{
						SSL_LOG_WRITE(ctx, line_mode::suffix, "");

						size_t const read_sz = r;
						read_status_t const rst = (read_sz == data_buf.size()) ? read_status::full : read_status::again;

						rd_consume_status_t rdc_status = tr_read::consume_buffer(ctx, buffer_ref(data_buf.data(), read_sz), rst);
						switch (rdc_status) {
							case rd_consume_status::more:
								continue;

							case rd_consume_status::loop_break:
							case rd_consume_status::closed:
								return rdc_status;
						}
					}
				}

				BOOST_ASSERT(!"can't be reached");
			}
		};

		struct write
		{
			template<class ContextT>
			static int write_buffer_to_ssl(ContextT *ctx, buffer_t *b)
			{
				CYASSL *ssl = ssl_from_context(ctx);

				while (!b->empty())
				{
					str_ref const buf_data = b->used_part();

					SSL_LOG_WRITE(ctx, line_mode::single, "write_to_ssl: {{ {0}, {1} }"
							, buf_data.size(), meow::format::as_hex_string(buf_data)
							);

					int r = CyaSSL_write(ssl, buf_data.data(), buf_data.size());

					if (r <= 0)
					{
						int err_code = CyaSSL_get_error(ssl, r);

						switch (err_code)
						{
							// it's fine, the writer might want to read
							//  in case we're in the middle of a handshake
							case SSL_ERROR_WANT_READ:
								return 0;

							// can recieve ssl shutdown message
							//  while in the middle of a handshake
							case SSL_ERROR_ZERO_RETURN:
								return 0;

							// should never happen, our write callback always
							//  accepts everything passed to it
							case SSL_ERROR_WANT_WRITE:
								return 0;

							default:
								return ssl->error;
						}
					}
					else
					{
						b->advance_first(r);
					}
				}

				return 0;
			}

			template<class ContextT>
			static int move_from_plaintext_to_wchain(ContextT *ctx)
			{
				buffer_chain_t& wchain = ctx->wchain_;
				buffer_chain_t& plaintext_wchain = ctx->w_plaintext_chain;

				if (!ctx_ssl_is_initialized(ctx))
				{
					wchain.append_chain(plaintext_wchain);
				}

				// write as much as possible to ssl
				while (!plaintext_wchain.empty())
				{
					buffer_t *b = plaintext_wchain.front();
					int ssl_errno = write_buffer_to_ssl(ctx, b);

					if (0 != ssl_errno)
					{
						char err_buf[1024] = {};
						CyaSSL_ERR_error_string_n(ssl_errno, err_buf, sizeof(err_buf));
						SSL_LOG_WRITE(ctx, line_mode::single, "SSL_write(): {0} - {1}", ssl_errno, err_buf);

						//ctx->cb_write_closed(io_close_report(io_close_reason::ssl_error, ssl_errno));
						return ssl_errno;
					}

					if (b->empty())
						plaintext_wchain.pop_front();
					else
						break;
				}

				return SSL_ERROR_NONE;
			}

			template<class ContextT>
			static wr_complete_status_t writev_bufs(ContextT *ctx)
			{
				int ssl_errno = move_from_plaintext_to_wchain(ctx);
				if (SSL_ERROR_NONE != ssl_errno)
				{
					//ctx->cb_write_closed(io_close_report(io_close_reason::ssl_error, ssl_errno));
					ctx->cb_write_closed(io_close_report(io_close_reason::custom_close));
					return wr_complete_status::closed;
				}

				return writev_from_wchain(ctx);
			}

			template<class ContextT>
			static wr_complete_status_t writev_from_wchain(ContextT *ctx)
			{
				buffer_chain_t& wchain = ctx->wchain_;
				io_context_t *io_ctx = &ctx->io_ctx_;

				IO_LOG_WRITE(ctx, line_mode::single, "{0}; ctx: {1}, wsz: {2}", __func__, ctx, wchain.size());

				if (wchain.empty())
					return wr_complete_status::finished;

				// check for just 1 buffer in the chain
				//  if that - use plain write() syscall
				if (wchain.end() == ++wchain.begin())
				{
					buffer_t *b = wchain.front();

					size_t offset = 0;
					size_t const total_size = b->used_size();

					while (offset < total_size)
					{
						size_t const wr_size = total_size - offset;

						IO_LOG_WRITE(ctx, line_mode::prefix
								, "::write({0}, {1} + {2}, {3} = {4} - {5}) = "
								, io_ctx->fd(), (void*)b->first, offset
								, wr_size, total_size, offset
								);

						ssize_t const n = ::write(io_ctx->fd(), b->first + offset, wr_size);

						if (-1 == n)
						{
							IO_LOG_WRITE(ctx, line_mode::suffix, "{0}, errno: {1} : {2}", n, errno, strerror(errno));

							if (EAGAIN == errno || EWOULDBLOCK == errno)
								break;

							ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
							return wr_complete_status::closed;
						}
						else
						{
							IO_LOG_WRITE(ctx, line_mode::suffix, "{0}", n);

							if (0 == n)
							{
								ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
								return wr_complete_status::closed;
							}

							offset += n;
						}
					}

					b->advance_first(offset);

					if (!b->empty())
						return wr_complete_status::more;

					wchain.pop_front();
					return wr_complete_status::finished;						
				}

				static size_t const writev_max_bufs = 8;
				struct iovec iov[writev_max_bufs];

				struct iovec *bufs = iov;
				size_t n_bufs = 0;

				typedef buffer_chain_t::iterator b_iter_t;
				b_iter_t b_i = wchain.begin();
				b_iter_t end_i = wchain.end();

				while (n_bufs > 0 || !wchain.empty())
				{
					// move the data to the beginning
					if (bufs != iov)
					{
						std::memmove(iov, bufs, n_bufs * sizeof(iov[0]));
						bufs = iov;
					}

					// fill up iovec as much as we can from the last known position
					if (n_bufs < writev_max_bufs)
					{
						while (b_i != end_i)
						{
							buffer_t *b = *b_i;

							bufs[n_bufs].iov_base = b->first;
							bufs[n_bufs].iov_len = b->used_size();

							++b_i;
							++n_bufs;

							if (n_bufs >= writev_max_bufs)
								break;
						}
					}

					IO_LOG_WRITE(ctx, line_mode::prefix, "::writev({0}, {1} : ", io_ctx->fd(), n_bufs);

					size_t total_len = 0;
					for (size_t i = 0; i < n_bufs; ++i)
					{
						struct iovec *v = bufs + i;
						total_len += v->iov_len;

						IO_LOG_WRITE(ctx, line_mode::middle, "{2}{{ {0}, {1} }"
								, v->iov_base, v->iov_len
								, ((i > 0) ? ", " : "")
								);
					}

					IO_LOG_WRITE(ctx, line_mode::middle, ", {0}) = ", total_len);
					ssize_t n = ::writev(io_ctx->fd(), bufs, n_bufs);
					if (n >= 0)
						IO_LOG_WRITE(ctx, line_mode::suffix, "{0}", n);
					else
						IO_LOG_WRITE(ctx, line_mode::suffix, "{0}; {1} - {2}", n, errno, strerror(errno));

					switch (n)
					{
					case -1:
						if (EAGAIN == errno || EWOULDBLOCK == errno)
						{
							return wr_complete_status::more;
						}
						else
						{
							ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
							return wr_complete_status::closed;
						}

					case 0:
						ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
						return wr_complete_status::closed;

					default:
						size_t len = n;
						while (len > 0 && len >= bufs->iov_len)
						{
							len -= bufs->iov_len;
							bufs++;

							// we assume that writev() never returns more than all the buffers
							//  could hold, so n_bufs can't underflow here
							BOOST_ASSERT(n_bufs > 0);
							n_bufs--;

							// buf fully written
							wchain.pop_front();
						}

						if (len > 0)
						{
							bufs->iov_base = (char*)bufs->iov_base + len;
							bufs->iov_len -= len;

							// partial write, adjust anyway, might not have the chance later
							buffer_t *b = wchain.front();
							b->advance_first(len);
						}
						break;
					} // switch
				} // while

				return wr_complete_status::finished;
			}
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct ssl_connection_t;

	struct ssl_connection_events_t
	{
		virtual ~ssl_connection_events_t() {}

		virtual buffer_ref           get_buffer(ssl_connection_t*) = 0;
		virtual rd_consume_status_t  consume_buffer(ssl_connection_t*, buffer_ref, bool is_closed) = 0;
		virtual void                 on_ssl_error(ssl_connection_t*, int ev_mask, int ssl_errno) = 0;
		virtual void                 on_closed(ssl_connection_t*, io_close_report_t const&) = 0;
	};

	struct ssl_connection_t : public generic_connection_t
	{
		virtual ~ssl_connection_t() {}

		virtual void ssl_init(CYASSL_CTX*) = 0;
		virtual bool ssl_is_initialized() const = 0;
		virtual void ssl_connect() = 0;
		virtual void ssl_accept() = 0;
	};

	template<class EventsT>
	struct ssl_connection_e_t : public ssl_connection_t
	{
		typedef EventsT events_t;

		virtual events_t* ev() const = 0;
		virtual void      set_ev(events_t*) = 0;
	};

	template<
		  class Traits
		, class Interface = ssl_connection_t
		>
	struct ssl_connection_impl_t
		: public generic_connection_impl_t<Interface, ssl_connection_repack_traits<Traits> >
		, public cyassl_custom_io_ctx_t
	{
		typedef generic_connection_impl_t<Interface, ssl_connection_repack_traits<Traits> > impl_t;
		typedef typename impl_t::events_t  events_t;
		typedef typename ssl_connection_repack_traits<Traits>::ssl_log_writer ssl_log_writer;

	public:

		ssl_connection_impl_t(evloop_t *loop, int fd, events_t *ev = NULL)
			: impl_t(loop, fd, ev)
		{
		}

		virtual void queue_buf(buffer_move_ptr buf)
		{
			if (!buf || buf->empty())
				return;

			this->w_plaintext_chain.push_back(move(buf));
		}

		virtual void queue_chain(buffer_chain_t& chain)
		{
			this->w_plaintext_chain.append_chain(chain);
		}

	public:

		virtual void ssl_init(CYASSL_CTX *ssl_ctx)
		{
			CYASSL *ssl = CyaSSL_new(ssl_ctx);
			if (NULL == ssl)
				throw std::logic_error("CyaSSL_new() failed: make sure you've got certificate/private-key and memory");

			this->rw_ssl.reset(ssl);
			CyaSSL_SetIOReadCtx(this->rw_ssl.get(), static_cast<cyassl_custom_io_ctx_t*>(this));
			CyaSSL_SetIOWriteCtx(this->rw_ssl.get(), static_cast<cyassl_custom_io_ctx_t*>(this));
		}

		virtual bool ssl_is_initialized() const
		{
			return !!this->rw_ssl;
		}

		virtual void ssl_connect()
		{
			BOOST_ASSERT(this->ssl_is_initialized());
			CyaSSL_set_connect_state(this->rw_ssl.get());
		}

		virtual void ssl_accept()
		{
			BOOST_ASSERT(this->ssl_is_initialized());
			CyaSSL_set_accept_state(this->rw_ssl.get());
		}

	private:

		virtual int cyassl_read(char *buf, int buf_sz)
		{
			SSL_LOG_WRITE(this, line_mode::prefix, "{0}; to buf_sz: {1} ", __func__, buf_sz);

			BOOST_ASSERT(buf_sz > 0);

			buffer_move_ptr& b = this->ssl_rbuf;

			if (!b)
			{
				SSL_LOG_WRITE(this, line_mode::suffix, "<-- IO_ERR_WANT_READ [no buf]");
				return IO_ERR_WANT_READ;
			}

			if (b->empty())
			{
				b->clear();
				SSL_LOG_WRITE(this, line_mode::suffix, "<-- IO_ERR_WANT_READ [empty]");
				return IO_ERR_WANT_READ;
			}

			size_t const sz = MIN((size_t)buf_sz, b->used_size());
			std::memcpy(buf, b->first, sz);
			b->advance_first(sz);

			SSL_LOG_WRITE(this, line_mode::suffix, "<-- {{ {0}, {1} }", sz, meow::format::as_hex_string(str_ref(buf, sz)));
			return sz;
		}

		virtual int cyassl_write(char *buf, int buf_sz)
		{
			SSL_LOG_WRITE(this, line_mode::single, "{0}; buf: {{ {1}, {2} }", __func__, buf_sz, meow::format::as_hex_string(str_ref(buf, buf_sz)));

			if (buf_sz <= 0)
				return 0;

			buffer_move_ptr b = buffer_create_with_data(buf, buf_sz);
			this->wchain_.push_back(move(b));

#if 0
			static size_t const wr_buffer_size = 8 * 1024;

			buffer_chain_t& wchain = this->wchain_;

			for (size_t remaining_sz = buf_sz; remaining_sz > 0; /**/)
			{
				if (wchain.empty() || wchain.back()->full())
					wchain.push_back(create_buffer(wr_buffer_size));

				buffer_t *b = this->wchain_.back();

				size_t const sz = MIN(remaining_sz, b->free_size());
				copy_to_buffer(*b, buf, sz);

				remaining_sz -= sz;
			}
#endif
			return buf_sz;
		}

		#undef SSL_LOG_WRITE
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__SSL_CONNECTION_HPP_

