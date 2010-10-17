////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONVERT__WHATEVER_CAST_HPP_
#define MEOW_CONVERT__WHATEVER_CAST_HPP_

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class To, class From, class Enabler = void>
	struct whatever_caster_t;

	template<class To, class From>
	struct whatever_caster_t<To, From, typename boost::enable_if<boost::is_convertible<From, To> >::type>
	{
		void operator()(To& to, From const& from) const { to = from; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class To, class From>
	inline void whatever_assign(To& to, From const& from)
	{
		static whatever_caster_t<To, From> caster_;
		caster_(to, from);
	}

	template<class To, class From>
	inline To whatever_cast(From const& from)
	{
		static whatever_caster_t<To, From> caster_;
		To to;
		caster_(to, from);
		return to;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONVERT__WHATEVER_CAST_HPP_

