////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_STR_REF_TO_OSTREAM_HPP_
#define MEOW_STR_REF_TO_OSTREAM_HPP_

#include <iosfwd>
#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT, class Traits>
	inline
	std::basic_ostream<CharT, Traits>&
	operator<<(std::basic_ostream<CharT, Traits>& o, string_ref<CharT> const& s)
	{
		return o.write(s.data(), s.size());
	}

	template<class CharT, class Traits>
	inline
	std::basic_ostream<CharT, Traits>&
	operator<<(std::basic_ostream<CharT, Traits>& o, string_ref<CharT const> const& s)
	{
		return o.write(s.data(), s.size());
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_STR_REF_TO_OSTREAM_HPP_
