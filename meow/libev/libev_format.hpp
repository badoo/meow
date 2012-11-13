////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__LIBEV_FORMAT_HPP_
#define MEOW_LIBEV__LIBEV_FORMAT_HPP_

#include <ev.h>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/format/format_functions.hpp>
#include <meow/format/sink/char_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef tmp_buffer<128> evmask_tmp_buffer_t;

	inline str_ref as_evmask(int revents, evmask_tmp_buffer_t const& b = evmask_tmp_buffer_t())
	{
		sink::char_buffer_sink_t sink(b.get(), b.size());

	#define CHECKED_ADD(ev)				\
		do { if (ev & revents) {		\
			meow::format::write(sink	\
					, (!sink.size() ? ref_lit("") : ref_lit("|"))	\
					, ref_lit(#ev)		\
					); 					\
		} }	while (0)					\
	/**/

#if (3 == EV_VERSION_MAJOR)
		CHECKED_ADD(EV_READ);
		CHECKED_ADD(EV_WRITE);
		CHECKED_ADD(EV__IOFDSET);
		CHECKED_ADD(EV_TIMEOUT);
		CHECKED_ADD(EV_PERIODIC);
		CHECKED_ADD(EV_SIGNAL);
		CHECKED_ADD(EV_CHILD);
		CHECKED_ADD(EV_STAT);
		CHECKED_ADD(EV_IDLE);
		CHECKED_ADD(EV_PREPARE);
		CHECKED_ADD(EV_CHECK);
		CHECKED_ADD(EV_EMBED);
		CHECKED_ADD(EV_FORK);
		CHECKED_ADD(EV_ASYNC);
		CHECKED_ADD(EV_CUSTOM);
		CHECKED_ADD(EV_ERROR);

#elif (4 == EV_VERSION_MAJOR)
		CHECKED_ADD(EV_READ);
		CHECKED_ADD(EV_WRITE);
		CHECKED_ADD(EV__IOFDSET);
		CHECKED_ADD(EV_TIMER);
		CHECKED_ADD(EV_PERIODIC);
		CHECKED_ADD(EV_SIGNAL);
		CHECKED_ADD(EV_CHILD);
		CHECKED_ADD(EV_STAT);
		CHECKED_ADD(EV_IDLE);
		CHECKED_ADD(EV_PREPARE);
		CHECKED_ADD(EV_CHECK);
		CHECKED_ADD(EV_EMBED);
		CHECKED_ADD(EV_FORK);
		CHECKED_ADD(EV_CLEANUP);
		CHECKED_ADD(EV_ASYNC);
		CHECKED_ADD(EV_CUSTOM);
		CHECKED_ADD(EV_ERROR);
#endif

	#undef CHECKED_ADD

		return sink.used_part();
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__LIBEV_FORMAT_HPP_

