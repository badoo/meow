////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__FD_CONNECTOR_HPP_
#define MEOW_LIBEV__FD_CONNECTOR_HPP_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <meow/unix/ipv4_address.hpp>
#include <meow/unix/time.hpp> 	// os_timeval_t

#include <meow/move_ptr/static_move_ptr.hpp>

#include <meow/libev/libev_fwd.hpp>
#include <meow/libev/io_context.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef boost::static_move_ptr<io_context_t> io_context_ptr;

	struct fd_connector_t : private boost::noncopyable
	{
		typedef void* token_t;
		typedef boost::function<void(io_context_ptr& io_ctx, int err)> callback_t;

		virtual ~fd_connector_t() {}

		virtual evloop_t* loop() const = 0;

		virtual token_t try_connect(
				  callback_t const& 		cb
				, int 						fd
				, ipv4::address_t const& 	addr
				, os_timeval_t const& 		timeout
			) = 0;

		virtual void cancel_connect(
				  token_t  token
				, bool 	   do_callback = true
			) = 0;
	};

	typedef boost::static_move_ptr<fd_connector_t> fd_connector_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__FD_CONNECTOR_HPP_

