////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__HEX_STRING_HPP_
#define MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

#include <string>

#include <boost/type_traits/remove_const.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	inline
	std::basic_string<typename boost::remove_const<CharT>::type>
	as_hex_string(string_ref<CharT> const& s)
	{
		std::basic_string<typename boost::remove_const<CharT>::type> result(s.size() * 2, CharT());

		CharT *ee = copy_bin2hex(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result;
	}

	template<size_t N>
	inline
	str_ref as_hex_string(str_ref const& s, meow::tmp_buffer<N*2> const& b = meow::tmp_buffer<N*2>())
	{
		BOOST_ASSERT(s.size() <= N);

		char const *ee = meow::copy_bin2hex(s.begin(), s.end(), b.begin());
		BOOST_ASSERT(ee <= b.end());

		return str_ref(b.begin(), ee);
	}

	template<class CharT>
	inline
	std::basic_string<typename boost::remove_const<CharT>::type>
	as_escaped_hex_string(string_ref<CharT> const& s)
	{
		std::basic_string<typename boost::remove_const<CharT>::type> result(s.size() * 4, CharT());

		CharT *ee = copy_bin2hex_escaped(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

