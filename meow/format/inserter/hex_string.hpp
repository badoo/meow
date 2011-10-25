////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__HEX_STRING_HPP_
#define MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

#include <string>

#include <meow/str_ref.hpp>
#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	std::basic_string<CharT> as_hex_string(string_ref<CharT> const& s)
	{
		std::basic_string<CharT> result(s.size() * 2, CharT());

		CharT *ee = copy_bin2hex(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result;
	}

	template<class CharT>
	std::basic_string<CharT> as_escaped_hex_string(string_ref<CharT> const& s)
	{
		std::basic_string<CharT> result(s.size() * 4, CharT());

		CharT *ee = copy_bin2hex_escaped(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

