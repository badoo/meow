////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__HEX_STRING_HPP_
#define MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

#include <type_traits>

#include <meow/str_ref.hpp>
#include <meow/str_copy.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	inline
	meow::string_copy<typename std::remove_const<CharT>::type>
	as_hex_string(meow::string_ref<CharT> const& s)
	{
		meow::string_copy<typename std::remove_const<CharT>::type> result(s.size() * 2);

		CharT *ee = copy_bin2hex(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result; // rely on NRVO here, simple return by value is faster than move()
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
	meow::string_copy<typename std::remove_const<CharT>::type>
	as_escaped_hex_string(meow::string_ref<CharT> const& s)
	{
		meow::string_copy<typename std::remove_const<CharT>::type> result(s.size() * 4);

		CharT *ee = copy_bin2hex_escaped(s.begin(), s.end(), &*result.begin());
		BOOST_ASSERT(ee == &*result.end());

		return result; // rely on NRVO here, simple return by value is faster than move()
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

