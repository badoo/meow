////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONVERT__WHATEVER_FROM_STR_REF_HPP_
#define MEOW_CONVERT__WHATEVER_FROM_STR_REF_HPP_

#include <boost/lexical_cast.hpp> // blergh

#include <meow/str_ref.hpp>
#include <meow/str_ref_to_ostream.hpp>

#include "number_from_string.hpp"
#include "whatever_cast.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct whatever_caster_t<str_ref, str_ref>
	{
		void operator()(str_ref& to, str_ref const& from) const { to = from; }
	};

	template<>
	struct whatever_caster_t<char const*, str_ref>
	{
		void operator()(char const*& to, str_ref const& from) const { to = from.data(); }
	};

	template<class CharT>
	struct whatever_caster_t<std::string, string_ref<CharT> >
	{
		void operator()(std::string& to, string_ref<CharT const> const& from) const
		{
			to.assign(from.begin(), from.end());
		}
	};

	template<class T>
	struct whatever_caster_t<
			  T
			, str_ref
			, typename boost::enable_if<boost::is_arithmetic<T> >::type
			>
	{
		void operator()(T& to, str_ref const& from) const { to = number_from_string<T>(from); }
	};

	template<>
	struct whatever_caster_t<bool, str_ref>
	{
		void operator()(bool& to, str_ref const& from) const
		{
			if (   (ref_lit("1") == from)
				|| (ref_lit("yes") == from)
				|| (ref_lit("true") == from)
				)
			{
				to = true;
			}
			else
			{
				to = false;
			}
		}
	};

	template<class T, class CharT>
	struct whatever_caster_t<
		  T
		, string_ref<CharT>
		, typename boost::disable_if<boost::is_arithmetic<T> >::type
		>
	{
		void operator()(T& to, string_ref<CharT const> const& from) const { to = boost::lexical_cast<T>(from); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONVERT__WHATEVER_FROM_STR_REF_HPP_

