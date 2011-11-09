////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_
#define MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_

#include <meow/str_ref.hpp>

#include <meow/libev/io_close_report.hpp>
#include <meow/libev/detail/generic_connection.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct bin_msg_connection_t : public generic_connection_t
	{
		struct events_t
		{
			virtual ~events_t() {}

			// return -1 from this function if header is bad and you've aborted the connection
			virtual ssize_t on_header(bin_msg_connection_t*, str_ref const& headers_data) = 0;
			virtual void on_message(bin_msg_connection_t*, buffer_move_ptr) = 0;
			virtual void on_closed(bin_msg_connection_t*, io_close_report_t const&) = 0;
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BIN_MSG_CONNECTION_HPP_

