////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__HEX_STRING_HPP_
#define MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

#include <string>

#include <boost/range/iterator_range.hpp>

#include <meow/str_ref.hpp>
#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct hex_string_wrapper_t
	{
		string_ref<CharT const> str;
	};

	template<class CharT>
	inline hex_string_wrapper_t<CharT> as_hex_string(string_ref<CharT> const& s)
	{
		hex_string_wrapper_t<CharT> r;
		r.str = s;
		return r;
	}

	template<class CharT>
	struct type_tunnel<hex_string_wrapper_t<CharT> >
	{
		typedef typename boost::remove_const<CharT>::type string_char_t;
		typedef std::basic_string<string_char_t> string_t;

		static string_t call(hex_string_wrapper_t<CharT> const& s)
		{
			string_t result(s.str.size() * 2, string_char_t());

			CharT *ee = copy_bin2hex(s.str.begin(), s.str.end(), &*result.begin());
			BOOST_ASSERT(ee == &*result.end());

			return result;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct hex_escaped_string_wrapper_t
	{
		string_ref<CharT const> str;
	};

	template<class CharT>
	inline hex_escaped_string_wrapper_t<CharT> as_escaped_hex_string(string_ref<CharT> const& s)
	{
		hex_escaped_string_wrapper_t<CharT> r;
		r.str = s;
		return r;
	}

	template<class CharT>
	struct type_tunnel<hex_escaped_string_wrapper_t<CharT> >
	{
		typedef typename boost::remove_const<CharT>::type string_char_t;
		typedef std::basic_string<string_char_t> string_t;

		static string_t call(hex_escaped_string_wrapper_t<CharT> const& s)
		{
			string_t result(s.str.size() * 4, string_char_t());

			CharT *ee = copy_bin2hex_escaped(s.str.begin(), s.str.end(), &*result.begin());
			BOOST_ASSERT(ee == &*result.end());

			return result;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__HEX_STRING_HPP_

