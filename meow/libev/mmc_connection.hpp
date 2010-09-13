////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>

#include <meow/buffer.hpp>
#include <meow/str_ref.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

#include "io_close_report.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct mmc_connection_t : private boost::noncopyable
	{
		virtual ~mmc_connection_t() {}

		struct events_t
		{
			virtual ~events_t() {}

			virtual void on_message(mmc_connection_t*, str_ref) = 0;
			virtual void on_reader_error(mmc_connection_t*, str_ref) = 0;
			virtual void on_closed(mmc_connection_t*, io_close_report_t const&) = 0;
		};

		virtual int fd() const = 0;

		virtual void activate() = 0;
		virtual void send(buffer_move_ptr) = 0;
		virtual void close_after_write() = 0;
		virtual void close_immediately() = 0;
	};

	typedef boost::static_move_ptr<mmc_connection_t> mmc_connection_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_HPP_

