////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_
#define MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_

#include <meow/format/format_to_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class C, class F, class... A>
	inline void connection_queue_fmt_l(C *c, size_t init_sz, F const& f, A const&... args)
	{
		c->queue_buf(format::fmt_buf(init_sz, f, args...));
	}

	template<class C, class F, class... A>
	inline void connection_queue_fmt(C *c, F const& f, A const&... args)
	{
		connection_queue_fmt_l(c, 256, f, args...);
	}

	template<class C, class F, class... A>
	inline void connection_send_fmt(C *c, F const& f, A const&... args)
	{
		connection_queue_fmt_l(c, 256, f, args...);
		c->w_activate();
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_

