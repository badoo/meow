////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_HPP_

#include <meow/str_ref.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

#include <meow/libev/io_close_report.hpp>
#include <meow/libev/detail/generic_connection.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct mmc_reader_events_t
	{
		typedef generic_connection_t connection_t;

		virtual ~mmc_reader_events_t() {}

		virtual bool on_message(connection_t*, str_ref const& message) = 0;
		virtual void on_error(connection_t*, str_ref const& error_msg) = 0;
		virtual void on_closed(connection_t*, io_close_report_t const&) = 0;
	};

	typedef generic_connection_e_t<mmc_reader_events_t> mmc_connection_t;
	typedef boost::static_move_ptr<mmc_connection_t> mmc_connection_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_HPP_

