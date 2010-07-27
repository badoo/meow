////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_DETAIL__INTEGER_TRAITS_HPP_
#define MEOW_FORMAT_DETAIL__INTEGER_TRAITS_HPP_

#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	// metafunction telling us AT MOST how many characters
	//  will printing the number of
	//    - size 'n_bits' bits
	//    - radix 'radix'
	//  take
	//  this includes the terminating 0 and sign, just in case
	template<size_t n_bits, size_t radix>
	struct number_buffer_max_length;

	template<size_t n_bits>
	struct number_buffer_max_length<n_bits, 2> { enum { value = 65 }; };

	template<size_t n_bits>
	struct number_buffer_max_length<n_bits, 8> { enum { value = 23 }; };

	template<size_t n_bits>
	struct number_buffer_max_length<n_bits, 10> { enum { value = 21 }; };

	template<size_t n_bits>
	struct number_buffer_max_length<n_bits, 16> { enum { value = 17 }; };

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_DETAIL__INTEGER_TRAITS_HPP_

