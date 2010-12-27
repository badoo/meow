////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__LIBEV_HPP_
#define MEOW_LIBEV__LIBEV_HPP_

#include <ev.h>
#include "libev_fwd.hpp"

#include <meow/movable_handle.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct pointer_handle_traits
	{
		typedef T handle_type;
		static handle_type null() { return NULL; }
		static bool is_null(handle_type const& h) { return null() == h; }
	};

	struct ev_loop_default_traits : public pointer_handle_traits<struct ev_loop*>
	{
		static void close(handle_type& h) { /*printf("%s\n", __PRETTY_FUNCTION__);*/ ev_default_destroy(); }
	};
	typedef meow::movable_handle<ev_loop_default_traits> evloop_default_t;

	struct ev_loop_dynamic_traits : public pointer_handle_traits<struct ev_loop*>
	{
		static void close(handle_type& h) { ev_loop_destroy(h); }
	};
	typedef meow::movable_handle<ev_loop_dynamic_traits> evloop_dynamic_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	// don't call this more than once, unless you know what you're doing
	inline evloop_default_t create_default_loop(unsigned int flags)
	{
		return evloop_default_t(ev_default_loop(flags));
	}

	inline evloop_dynamic_t create_loop(unsigned int flags)
	{
		return evloop_dynamic_t(ev_loop_new(flags));
	}

	template<class L>
	inline void run_loop(L& loop, unsigned int flags = 0)
	{
		ev_loop(get_handle(loop), flags);
	}

	template<class L>
	inline void break_loop(L& loop, unsigned int flags = 0)
	{
		ev_unloop(get_handle(loop), flags);
	}

	inline void break_loop(evloop_t *loop, unsigned int flags = 0)
	{
		ev_unloop(get_handle(loop), flags);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__LIBEV_HPP_

