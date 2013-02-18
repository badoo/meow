////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2012 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_
#define MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_

#include <cstdarg> // va_arg
#include <cstddef> // size_t
#include <cstdio>  // vsnprintf

#include <boost/assert.hpp>

#include <meow/config/compiler_features.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////


	inline std::string as_printf(char const *fmt, ...) MEOW_PRINTF_LIKE(1,2);

	inline std::string as_printf(char const *fmt, ...)
	{
		va_list ap;

		va_start(ap, fmt);
		ssize_t const real_buffer_size = ::vsnprintf(NULL, 0, fmt, ap) + 1 /* trailing 0x0 */;
		BOOST_ASSERT(real_buffer_size > 0);
		va_end(ap);

		va_start(ap, fmt);
		std::string result(real_buffer_size, char());
		ssize_t n = ::vsnprintf(&*result.begin(), result.size(), fmt, ap);
		BOOST_ASSERT((n + 1) == real_buffer_size);
		va_end(ap);

		result.resize(n); // cut out the trailing null-char, don't want it there
		return result;
	}

	template<size_t N>
	struct fmt_buffer_t
	{
		fmt_buffer_t() : length(0) {}

		size_t  length;
		char    data[N];
	};

	template<size_t N>
	struct string_access<fmt_buffer_t<N> >
	{
		static str_ref call(fmt_buffer_t<N> const& b)
		{
			return str_ref(b.data, b.length);
		}
	};

	template<size_t N>
	inline fmt_buffer_t<N> as_printf_tmp(char const *fmt, ...) MEOW_PRINTF_LIKE(1,2);

	template<size_t N>
	inline fmt_buffer_t<N> as_printf_tmp(char const *fmt, ...)
	{
		fmt_buffer_t<N> result;

		va_list ap;

		va_start(ap, fmt);
		ssize_t n = ::vsnprintf(result.data, N, fmt, ap);
		BOOST_ASSERT(n <= N);
		va_end(ap);

		result.length = n;
		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__AS_PRINTF_HPP_

