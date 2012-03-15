////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__IO_CLOSE_REPORT_HPP_
#define MEOW_LIBEV__IO_CLOSE_REPORT_HPP_

#include <meow/smart_enum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	// low level io-related close reason
	MEOW_DEFINE_SMART_ENUM(io_close_reason, ((io_error, 		"io_error"))
											((ssl_error,        "ssl coding error"))
											((peer_close, 		"peer closed connection"))
											((write_close, 		"server write and close"))
											((custom_close, 	"server immediate close"))
											((sync_close,       "server syncronous close"))
											);

	struct io_close_report_t
	{
		io_close_reason_t 	reason;
		int 				code;

		io_close_report_t(io_close_reason_t r, int c = 0)
			: reason(r)
			, code(c)
		{
		}
	};

	inline io_close_report_t io_close_report(io_close_reason_t r, int c = 0)
	{
		return io_close_report_t(r, c);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__IO_CLOSE_REPORT_HPP_

