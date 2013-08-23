////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2012 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_
#define MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_

#include <cstdarg> // va_arg
#include <cstddef> // size_t
#include <cstdio>  // vsnprintf

#include <meow/config/compiler_features.hpp>
#include <meow/format/sink/char_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////


	inline std::string as_printf(char const *fmt, ...) MEOW_PRINTF_LIKE(1,2);

	inline std::string as_printf(char const *fmt, ...)
	{
		va_list ap;

		va_start(ap, fmt);
		ssize_t const real_buffer_size = ::vsnprintf(NULL, 0, fmt, ap) + 1 /* trailing 0x0 */;
		assert(real_buffer_size > 0);
		va_end(ap);

		va_start(ap, fmt);
		std::string result(real_buffer_size, char());
		ssize_t n = ::vsnprintf(&*result.begin(), result.size(), fmt, ap);
		assert((n + 1) == real_buffer_size);
		va_end(ap);

		result.resize(n); // cut out the trailing null-char, don't want it there
		return result;
	}

	template<size_t N>
	inline stack_buffer_t<N> as_printf_tmp(char const *fmt, ...) MEOW_PRINTF_LIKE(1,2);

	template<size_t N>
	inline stack_buffer_t<N> as_printf_tmp(char const *fmt, ...)
	{
		stack_buffer_t<N> result;

		va_list ap;

		va_start(ap, fmt);
		ssize_t n = ::vsnprintf(result.data, N, fmt, ap);
		assert(n <= N);
		va_end(ap);

		result.length = n;

		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_

