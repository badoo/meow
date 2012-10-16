////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2012 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_
#define MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/format/format.hpp>
#include <meow/format/inserter/hex_string.hpp>

#include "base_types.hpp"
#include "../detail/generic_connection.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef SSL_CTX openssl_ctx_t;

	struct SSL_CTX_deleter_t {
		void operator()(openssl_ctx_t *ssl_ctx) { SSL_CTX_free(ssl_ctx); }
	};
	typedef boost::static_move_ptr<openssl_ctx_t, SSL_CTX_deleter_t> openssl_ctx_move_ptr;

	inline openssl_ctx_move_ptr openssl_ctx_create(SSL_METHOD const *method)
	{
		return openssl_ctx_move_ptr(SSL_CTX_new(method));
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef SSL openssl_t;

	struct SSL_deleter_t {
		void operator()(openssl_t *ssl) { SSL_free(ssl); }
	};
	typedef boost::static_move_ptr<openssl_t, SSL_deleter_t> openssl_move_ptr;

	inline openssl_move_ptr openssl_create(openssl_ctx_t *ctx)
	{
		return openssl_move_ptr(SSL_new(ctx));
	}

	inline int openssl_last_error_code()
	{
		return ERR_get_error();
	}

	typedef meow::tmp_buffer<256> openssl_err_buf_t;
	inline str_ref openssl_get_error_string(int err_code, openssl_err_buf_t const& buf = openssl_err_buf_t())
	{
		format::sink::char_buffer_sink_t sink(buf.get(), buf.size());
		format::write(sink, ERR_func_error_string(err_code), ": ", ERR_reason_error_string(err_code));
		return sink.used_part();
	}

	inline str_ref openssl_get_last_error_string(openssl_err_buf_t const& buf = openssl_err_buf_t())
	{
		return openssl_get_error_string(openssl_last_error_code(), buf);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct openssl_connection_t : public generic_connection_t
	{
		virtual ~openssl_connection_t() {}

		virtual void ssl_init(openssl_ctx_t*) = 0;
		virtual bool ssl_is_initialized() const = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#include "../detail/generic_connection_impl.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct openssl_connection_repack_traits : public Traits
	{
		typedef openssl_t         ssl_t;
		typedef openssl_move_ptr  ssl_move_ptr;

		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, ssl_log_writer, ssl_log_writer_traits__default);

		// this shit needs to be defined here
		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, base, generic_connection_traits__base<Traits>);

		typedef typename Traits::read tr_read;

		struct rw_context_t : public tr_read::context_t
		{
			buffer_move_ptr  ssl_rbuf;
			buffer_chain_t   w_plaintext_chain;
			ssl_move_ptr     rw_ssl;
		};

		template<class ContextT>
		static ssl_t* ssl_from_context(ContextT *ctx)
		{
			return ctx->rw_ssl.get();
		}

		struct read
		{
			typedef rw_context_t context_t; // needed by generic_connection_impl_t
			static size_t const network_buffer_size = 16 * 1024;

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				if (NULL == ssl_from_context(ctx))
					return tr_read::get_buffer(ctx);

				buffer_move_ptr& b = ctx->ssl_rbuf;
				if (!b)
					b = create_buffer(network_buffer_size);

				return b->free_part();
			}

			template<class ContextT>
			static rd_consume_status_t consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
			{
				ssl_t *ssl = ssl_from_context(ctx);

				if (NULL == ssl)
					return tr_read::consume_buffer(ctx, read_part, r_status);

				if (read_status::error == r_status)
					return tr_read::consume_buffer(ctx, buffer_ref(), r_status);

				if (read_status::again == r_status && read_part.empty())
					return rd_consume_status::loop_break;

				buffer_move_ptr& b = ctx->ssl_rbuf;
				b->advance_last(read_part.size());

				SSL_LOG_WRITE(ctx, line_mode::single, "{0}; {1} - {{ {2}, {3} }"
						, __func__, read_status::enum_as_str_ref(r_status)
						, read_part.size(), meow::format::as_hex_string(read_part));

				while (true)
				{
					buffer_ref data_buf = tr_read::get_buffer(ctx);

					if (!data_buf)
						return rd_consume_status::loop_break;

					int r = SSL_read(ssl, data_buf.data(), data_buf.size());
					SSL_LOG_WRITE(ctx, line_mode::prefix, "SSL_read({0}, {1}, {2}) = {3}", ctx, (void*)data_buf.data(), data_buf.size(), r);

					if (r <= 0)
					{
						int err_code = ERR_get_error();
						if (err_code)
							SSL_LOG_WRITE(ctx, line_mode::suffix, ", ssl_code: {0} - {1}", err_code, openssl_get_error_string(err_code));
						else
							SSL_LOG_WRITE(ctx, line_mode::suffix, "");

						int ssl_code = SSL_get_error(ssl, r);
						switch (ssl_code)
						{
							case SSL_ERROR_ZERO_RETURN:
								return tr_read::consume_buffer(ctx, buffer_ref(), read_status::closed);

							case SSL_ERROR_WANT_READ:
							case SSL_ERROR_WANT_WRITE:
								if (read_status::closed == r_status)
									return tr_read::consume_buffer(ctx, buffer_ref(), r_status);
								else
									return rd_consume_status::loop_break;

							default:
								return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
						}
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

		struct write : public generic_connection_traits__write<base>
		{
			template<class ContextT>
			static int write_buffer_to_ssl(ContextT *ctx, buffer_t *b)
			{
				ssl_t *ssl = ssl_from_context(ctx);

				while (!b->empty())
				{
					str_ref const buf_data = b->used_part();

					SSL_LOG_WRITE(ctx, line_mode::single, "{0}: {{ {1}, {2} }", __func__, buf_data.size(), meow::format::as_hex_string(buf_data));

					int r = SSL_write(ssl, buf_data.data(), buf_data.size());

					if (r <= 0)
					{
						int err_code = ERR_get_error();
						if (err_code)
							SSL_LOG_WRITE(ctx, line_mode::single, "SSL_write(): {0} - {1}", err_code, openssl_get_error_string(err_code));
						else
							SSL_LOG_WRITE(ctx, line_mode::suffix, "");

						int ssl_code = SSL_get_error(ssl, r);
						switch (ssl_code)
						{
							// can recieve ssl shutdown message while in the middle of a handshake
							case SSL_ERROR_ZERO_RETURN:
								return 0;

							// it's fine, the writer might want to read, in case we're in the middle of a handshake
							case SSL_ERROR_WANT_READ:
								return 0;

							// should never happen, our write callback always accepts everything passed to it
							case SSL_ERROR_WANT_WRITE:
								return 0;

							default:
								return -1;
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
			static int move_wchain_buffers_from_to(ContextT *ctx, buffer_chain_t& from, buffer_chain_t *to)
			{
				// ssl not enabled
				if (NULL == ssl_from_context(ctx))
				{
					to->append_chain(from);
					return 0;
				}

				// write as much as possible to ssl
				while (!from.empty())
				{
					buffer_t *b = from.front();
					int r = write_buffer_to_ssl(ctx, b);

					if (0 != r)
						return r;

					if (b->empty())
						from.pop_front();
					else
						break;
				}

				return 0;
			}

			template<class ContextT>
			static wr_complete_status_t writev_bufs(ContextT *ctx)
			{
				int ssl_errno = move_wchain_buffers_from_to(ctx, ctx->w_plaintext_chain, &ctx->wchain_);
				if (0 != ssl_errno)
				{
					ctx->cb_write_closed(io_close_report(io_close_reason::custom_close));
					return wr_complete_status::closed;
				}

				return writev_from_wchain(ctx, ctx->wchain_);
			}
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Traits
		, class Interface = generic_connection_t
		>
	struct openssl_connection_impl_t
		: public generic_connection_impl_t<Interface, openssl_connection_repack_traits<Traits> >
	{
		typedef openssl_connection_impl_t                 connection_t;
		typedef openssl_connection_repack_traits<Traits>  connection_traits;

		typedef openssl_ctx_t     ssl_ctx_t;
		typedef openssl_move_ptr  ssl_move_ptr;

		typedef generic_connection_impl_t<Interface, connection_traits> impl_t;
		typedef typename impl_t::events_t                   events_t;
		typedef typename connection_traits::ssl_log_writer  ssl_log_writer;

	public:

		openssl_connection_impl_t(evloop_t *loop, int fd, events_t *ev = NULL)
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

		virtual void ssl_init(ssl_ctx_t *ssl_ctx)
		{
			ssl_move_ptr ssl = openssl_create(ssl_ctx);
			if (!ssl)
				throw std::logic_error("ssl_create() failed: make sure you've got certificate/private-key and memory");

			static BIO_METHOD openssl_connection_bio_method =
			{
				BIO_TYPE_SOURCE_SINK,
				"openssl_connection_bio",
				connection_bio_t::bwrite,
				connection_bio_t::bread,
				connection_bio_t::bputs,
				connection_bio_t::bgets,
				connection_bio_t::ctrl,
				connection_bio_t::create,
				connection_bio_t::destroy,
				NULL
			};

			BIO *bio = BIO_new(&openssl_connection_bio_method);
			bio->ptr = this;
			SSL_set_bio(ssl.get(), bio, bio);

			SSL_set_accept_state(ssl.get());

			this->rw_ssl = move(ssl);
		}

		virtual bool ssl_is_initialized() const
		{
			return !!this->rw_ssl;
		}

	private:

		struct connection_bio_t
		{
			static int bwrite(BIO *bio, char const *buf, int buf_sz)
			{
				connection_t *c = (connection_t*)bio->ptr;

				SSL_LOG_WRITE(c, line_mode::single, "{0}; buf: {{ {1}, {2} }", __func__, buf_sz, meow::format::as_hex_string(str_ref(buf, buf_sz)));

				if (buf_sz <= 0)
					return 0;

				buffer_move_ptr b = buffer_create_with_data(buf, buf_sz);
				c->wchain_.push_back(move(b));

				return buf_sz;
			}

			static int bread(BIO *bio, char *buf, int buf_sz)
			{
				connection_t *c = (connection_t*)bio->ptr;

				SSL_LOG_WRITE(c, line_mode::prefix, "{0}; to buf_sz: {1} ", __func__, buf_sz);

				BOOST_ASSERT(buf_sz > 0);
				buffer_move_ptr& b = c->ssl_rbuf;

				if (!b)
				{
					SSL_LOG_WRITE(c, line_mode::suffix, "<-- WANT_READ [no buf]");
					BIO_set_retry_read(bio);
					return -1;
				}

				if (b->empty())
				{
					b->clear();
					SSL_LOG_WRITE(c, line_mode::suffix, "<-- WANT_READ [empty]");
					BIO_set_retry_read(bio);
					return -1;
				}

				BIO_clear_retry_flags(bio);

				size_t const sz = std::min((size_t)buf_sz, b->used_size());
				std::memcpy(buf, b->first, sz);
				b->advance_first(sz);

				SSL_LOG_WRITE(c, line_mode::suffix, "<-- {{ {0}, {1} }", sz, meow::format::as_hex_string(str_ref(buf, sz)));
				return sz;
			}

			static int bputs(BIO *bio, char const *str) { return -1; }
			static int bgets(BIO *bio, char *buf, int buf_sz) { return -1; }

			static int create(BIO *bio)
			{
				bio->init = 1;
				bio->shutdown = 1; 
				bio->num = 0; 
				bio->ptr = NULL;
				bio->flags = 0;
				return 1;
			}

			static int destroy(BIO *bio)
			{
				bio->init = 0;
				bio->ptr = NULL;
				return 1;
			}

			static long ctrl(BIO *bio, int cmd, long num, void *ptr)
			{
				switch (cmd) {
					case BIO_CTRL_FLUSH: return 1;
					default:             return 0;
				}
			}

		//	static long callback_ctrl(BIO *, int, bio_info_cb *) { return 0; }
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_

