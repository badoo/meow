////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__SSL_HPP_
#define MEOW_LIBEV_DETAIL__SSL_HPP_

#include <cyassl/ssl.h>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef CYASSL_CTX  ssl_ctx_t;
	typedef CYASSL      ssl_t;

	struct CYASSL_deleter_t {
		void operator()(ssl_t *ssl) { CyaSSL_free(ssl); }
	};
	typedef boost::static_move_ptr<ssl_t, CYASSL_deleter_t> ssl_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

	struct cyassl_custom_io_ctx_t
	{
		virtual ~cyassl_custom_io_ctx_t() {}
		virtual int cyassl_read(char *buf, int buf_sz) = 0;
		virtual int cyassl_write(char *buf, int buf_sz) = 0;

		static int cyassl_real_read_cb(char *buf, int buf_sz, void *ctx)
		{
			cyassl_custom_io_ctx_t *self = static_cast<cyassl_custom_io_ctx_t*>(ctx);
			return self->cyassl_read(buf, buf_sz);
		}

		static int cyassl_real_write_cb(char *buf, int buf_sz, void *ctx)
		{
			cyassl_custom_io_ctx_t *self = static_cast<cyassl_custom_io_ctx_t*>(ctx);
			return self->cyassl_write(buf, buf_sz);
		}
	};

	struct CyaSSL_CTX_deleter_t
	{
		void operator()(ssl_ctx_t *ssl_ctx) { CyaSSL_CTX_free(ssl_ctx); }
	};
	typedef boost::static_move_ptr<ssl_ctx_t, CyaSSL_CTX_deleter_t> ssl_ctx_move_ptr;

	inline ssl_ctx_move_ptr ssl_ctx_create(CYASSL_METHOD *method)
	{
		ssl_ctx_move_ptr h(CyaSSL_CTX_new(method));
		CyaSSL_SetIORecv(h.get(), &cyassl_custom_io_ctx_t::cyassl_real_read_cb);
		CyaSSL_SetIOSend(h.get(), &cyassl_custom_io_ctx_t::cyassl_real_write_cb);
		return move(h);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct cyassl_error_printer
	{
		size_t operator()(int error_code, char* buf, int buf_len) const
		{
			CyaSSL_ERR_error_string_n(error_code, buf, buf_len);
			return strlen(buf); // stupid ssl library, seriously :(
		}
	};

	typedef meow::api_call_error_ex<cyassl_error_printer> ssl_call_error_t;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__SSL_HPP_

