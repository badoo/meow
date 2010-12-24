////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_DETAIL__INTEGER_TO_STRING_HPP_
#define MEOW_FORMAT_DETAIL__INTEGER_TO_STRING_HPP_

#include <cstdlib>
#include <boost/assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_unsigned.hpp>

#include "radix_info.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_INT_TO_STRING_LOOP \
	do 							\
	{ 							\
		*--p = RadixI::get_character((CharT)0, value % RadixI::radix); \
		value /= RadixI::radix; \
	} 							\
	while(value); 				\
/**/

	// - will null-terminate the result
	// - the caller must supply sufficient length
	// returns: the pointer to first symbol of the result
	template<class RadixI, class CharT, class T>
	inline CharT* integer_to_string_ex(
			  CharT *buf
			, size_t const buf_sz
			, T value
			, typename boost::enable_if<boost::is_unsigned<T> >::type* = 0
			)
	{
		CharT *p = buf + buf_sz - 1; // last char

		*p = 0x0; // null terminate

		MEOW_INT_TO_STRING_LOOP;

		BOOST_ASSERT(buf <= p);
		return p;
	}

	template<class RadixI, class CharT, class T>
	inline CharT* integer_to_string_ex(
			  CharT *buf
			, size_t const buf_sz
			, T value
			, typename boost::enable_if<boost::is_signed<T> >::type* = 0
			)
	{
		CharT *p = buf + buf_sz - 1; // last char

		*p = 0x0; // null terminate

		if (value < 0)
		{
			MEOW_INT_TO_STRING_LOOP;
			*--p = CharT('-');
		}
		else
		{
			MEOW_INT_TO_STRING_LOOP;
		}

		BOOST_ASSERT(buf <= p);
		return p;
	}

	template<class CharT, class T>
	inline CharT* integer_to_string(CharT *buf, size_t const buf_sz, T value)
	{
		typedef radix_info_t<10> radix_t;
		return integer_to_string_ex<radix_t>(buf, buf_sz, value);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_DETAIL__INTEGER_TO_STRING_HPP_

