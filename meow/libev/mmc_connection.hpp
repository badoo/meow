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

	struct mmc_connection_t : public generic_connection_t
	{
		virtual ~mmc_connection_t() {}

		struct events_t
		{
			virtual ~events_t() {}

			virtual void on_message(mmc_connection_t*, str_ref) = 0;
			virtual void on_reader_error(mmc_connection_t*, str_ref) = 0;
			virtual void on_closed(mmc_connection_t*, io_close_report_t const&) = 0;
		};
	};

	typedef boost::static_move_ptr<mmc_connection_t> mmc_connection_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_HPP_

