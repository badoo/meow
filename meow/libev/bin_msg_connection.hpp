////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_
#define MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_

#include <boost/noncopyable.hpp>

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>
#include <meow/str_ref.hpp>

#include "io_close_report.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class HeaderT>
	struct bin_msg_connection_t : private boost::noncopyable
	{
		typedef bin_msg_connection_t 	self_t;
		typedef HeaderT 				header_t;

		virtual ~bin_msg_connection_t() {}

		struct events_t
		{
			virtual ~events_t() {}

			virtual void on_read(bin_msg_connection_t*, header_t const&, buffer_move_ptr) = 0;
			virtual void on_read_error(bin_msg_connection_t*, str_ref error_msg) = 0;
			virtual void on_closed(bin_msg_connection_t*, io_close_report_t const&) = 0;
		};

		virtual int fd() const = 0;

		virtual void activate() = 0;
		virtual void send(buffer_move_ptr) = 0;

		virtual void close_after_write() = 0;
		virtual void close_immediately() = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_

