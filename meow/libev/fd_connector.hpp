////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__FD_CONNECTOR_HPP_
#define MEOW_LIBEV__FD_CONNECTOR_HPP_

#include <functional>

#include <boost/noncopyable.hpp>

#include <meow/std_unique_ptr.hpp>
#include <meow/libev/libev_fwd.hpp>
#include <meow/libev/io_context.hpp>
#include <meow/unix/socket.hpp>
#include <meow/unix/time.hpp> 	// os_timeval_t

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef std::unique_ptr<io_context_t> io_context_ptr;

	struct fd_connector_t : private boost::noncopyable
	{
		typedef void* token_t;
		typedef std::function<void(io_context_ptr& io_ctx, int err)> callback_t;

		virtual ~fd_connector_t() {}

		virtual evloop_t* loop() const = 0;

		virtual token_t try_connect(
				  callback_t const&     cb
				, int 					fd
				, ev_tstamp const       timeout
				, os_sockaddr_t const 	*addr
				, os_socklen_t const    addr_len
			) = 0;

		virtual void cancel_connect(
				  token_t  token
				, bool 	   do_callback = true
			) = 0;
	};

	typedef std::unique_ptr<fd_connector_t> fd_connector_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__FD_CONNECTOR_HPP_

