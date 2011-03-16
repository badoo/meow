////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__LISTENER_HPP_
#define MEOW_LIBEV__LISTENER_HPP_

#include <boost/noncopyable.hpp>

#include <meow/libev/libev_fwd.hpp>
#include <meow/unix/ipv4_address.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

// listens on signle addr:port and calls back when a new connection is accepted
// XXX: can add another callback 'can_i_accept_new_connection_now?'. but that's not needed

	struct listener_t : private boost::noncopyable
	{
		virtual ~listener_t() {}

		virtual int 			fd() const = 0;
		virtual evloop_t* 		loop() const = 0;

		virtual void 			start(ipv4::address_t const&, int backlog = -1) = 0;
		virtual void 			shutdown() = 0;
	};

	typedef boost::static_move_ptr<listener_t> listener_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__LISTENER_HPP_

