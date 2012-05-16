////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__STATIC_MATH_HPP_
#define MEOW_UTILITY__STATIC_MATH_HPP_

#include <cstdlib> // for size_t

///////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
///////////////////////////////////////////////////////////////////////////////////////////////
// logarithms

	struct lower_bound_tag {};
	struct upper_bound_tag {};

	template<size_t val, size_t base, class tag = lower_bound_tag>
	struct static_logn {
		enum { value = static_logn<val/base, base, tag>::value + 1 };
	};

	template<size_t base> struct static_logn<1, base, lower_bound_tag> {
		enum { value = 0 };
	};

	template<size_t base> struct static_logn<1, base, upper_bound_tag> {
		enum { value = 1 };
	};
	
	template<size_t val, class tag = lower_bound_tag>
	struct static_log2 : public static_logn<val, 2, tag> {};

///////////////////////////////////////////////////////////////////////////////////////////////
// powers

	template<size_t val, size_t pow> struct static_pow {
		enum { value = val * static_pow<val, pow - 1>::value };
	};

	template<size_t val> struct static_pow<val, 0> {
		enum { value = 1 };
	};

///////////////////////////////////////////////////////////////////////////////////////////////
// is a given number 'val' a power of 'base_pow'

	template<size_t val, size_t base_pow> struct static_is_pow {
		enum { log = static_logn<val, base_pow, lower_bound_tag>::value };
		enum { value = (static_pow<base_pow, log>::value == val) };
	};

///////////////////////////////////////////////////////////////////////////////////////////////
// how to cover 'X bytes' with 'Y <Z_bytes_chunk>-s'

	template<size_t data_length, size_t cover_length>
	struct min_covering_length
	{
		enum { shift_by = static_log2<cover_length>::value };
		enum { and_mask = cover_length - 1 };

		static size_t const value = cover_length * ((data_length + (-data_length & and_mask)) >> shift_by);
	};

///////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
///////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__STATIC_MATH_HPP_

