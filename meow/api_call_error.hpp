////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_API_CALL_ERROR_HPP_
#define MEOW_API_CALL_ERROR_HPP_

#include <cerrno> 		// errno!
#include <cstdio>		// for snprintf
#include <cstring>		// for strerror_r
#include <cstdarg>		// for va_*
#include <exception>	// for std::exception

#include <meow/config/compiler_features.hpp>	// for MEOW_PRINTF_LIKE

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

struct simple_error_string
{
	size_t operator()(int error_code, char* buf, int buf_len) const
	{
		char error_desc[buf_len]; // VLA used

#if (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600) && !defined(_GNU_SOURCE)) \
	|| (defined(__APPLE__) || defined(__MACH__))
		// XSI compilant strerror_r
		if(0 == ::strerror_r(error_code, error_desc, buf_len))
			return ::snprintf(buf, buf_len, " - %s", error_desc);
#else
		// GNU strerror_r
		if(char const *p = ::strerror_r(error_code, error_desc, buf_len))
			return ::snprintf(buf, buf_len, " - %s", p);
#endif
		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail 
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ErrorPrinter	// prints strerror-like error message for given errno
		, size_t buffer_size_bytes = 256
	>
	struct api_call_error_base : public std::exception
	{
		static size_t const buffer_size = buffer_size_bytes;

		api_call_error_base(int code) : code_(code) {}

		int code() const throw() { return code_; }
		virtual char const* what() const throw() { return buffer_; }

	protected:
		void init(char const *fmt, va_list ap) throw()
		{
			size_t n = ::vsnprintf(buffer_, buffer_size, fmt, ap);

			if (n < buffer_size)
				n += snprintf(buffer_ + n, buffer_size - n, "; errno: %d(0x%08x)", code_, code_);

			if (n < buffer_size)
				n += err_printer_(code_, buffer_ + n, buffer_size - n);

			buffer_[buffer_size - 1] = 0x0;
		}

	private:
		int code_;
		char buffer_[buffer_size];
		ErrorPrinter err_printer_;
	};

	struct api_call_error : public api_call_error_base<detail::simple_error_string>
	{
		typedef api_call_error_base<detail::simple_error_string> base_type;

		// NOTE: disabled printf-like checks, because gcc-4.2-apple was broken with it
		//  		and was giving incorrect warnings for formats with more than 1 argument
		//  		and anyay we don't really need it much, as it's all generated
		explicit api_call_error(char const *fmt, ...) throw();// MEOW_PRINTF_LIKE(4,5);
		explicit api_call_error(int code, char const *fmt, ...) throw();// MEOW_PRINTF_LIKE(5,6);
	};

	template<class ErrorPrinter>
	struct api_call_error_ex : public api_call_error_base<ErrorPrinter>
	{
		typedef api_call_error_base<ErrorPrinter> base_type;

		// NOTE: disabled printf-like checks, because gcc-4.2-apple was broken with it
		//  		and was giving incorrect warnings for formats with more than 1 argument
		//  		and anyay we don't really need it much, as it's all generated
		explicit api_call_error_ex(char const *fmt, ...) throw();// MEOW_PRINTF_LIKE(4,5);
		explicit api_call_error_ex(int code, char const *fmt, ...) throw();// MEOW_PRINTF_LIKE(5,6);
	};

#define CTOR_BODY()	va_list ap; va_start(ap, fmt); this->init(fmt, ap);	va_end(ap); /**/

	inline api_call_error::api_call_error(char const *fmt, ...) throw() : base_type(errno) { CTOR_BODY(); }
	inline api_call_error::api_call_error(int code, char const *fmt, ...) throw() : base_type(code) { CTOR_BODY(); }

	template<class T>
	inline api_call_error_ex<T>::api_call_error_ex(char const *fmt, ...) throw() : base_type(errno) { CTOR_BODY(); }

	template<class T>
	inline api_call_error_ex<T>::api_call_error_ex(int code, char const *fmt, ...) throw() : base_type(code) { CTOR_BODY(); }

#undef CTOR_BODY

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_API_CALL_ERROR_HPP_

