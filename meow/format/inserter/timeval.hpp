////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__TIMEVAL_HPP_
#define MEOW_FORMAT_INSERTER__TIMEVAL_HPP_

#include <sys/time.h> // for timeval

#include <cstdio>

#include <boost/utility/enable_if.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>

#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct type_tunnel<struct timeval>
	{
		enum { buffer_size = sizeof("-1234567890.123456") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(struct timeval const& tv, buffer_t const& buf = buffer_t())
		{
			size_t n = ::snprintf(buf.get(), buf.size(), "%ld.%.6d", tv.tv_sec, static_cast<int>(tv.tv_usec));
			BOOST_ASSERT(n == (buffer_size - 1));
			return buf.get();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct timestamp_as_abs_t
	{
		time_t ts;
	};

	inline timestamp_as_abs_t as_abstime(time_t t)
	{
		timestamp_as_abs_t const r = { ts: t };
		return r;
	}

	template<>
	struct type_tunnel<timestamp_as_abs_t>
	{
		enum { buffer_size = sizeof("dd/mm/yyyy hh:mm:ss UTC") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(timestamp_as_abs_t const& t, buffer_t const& buf = buffer_t())
		{
			struct tm tm;
			::gmtime_r(&t.ts, &tm);

			ssize_t n = ::snprintf(
					  buf.get(), buf.size()
					, "%02u/%02u/%04u %02u:%02u:%02u UTC"
					, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900
					, tm.tm_hour, tm.tm_min, tm.tm_sec
				);

			BOOST_ASSERT(n == (buffer_size - 1));
			return buf.get();
		}
	};

	// this function is useful for converting the timestamp into a string in-place
	//  even if it looks somewhat unpleasanat on implementation
	typedef type_tunnel<timestamp_as_abs_t>::buffer_t format_abstime_b_t;
	inline str_ref format_abstime(time_t t, format_abstime_b_t const& buf = format_abstime_b_t())
	{
		return type_tunnel<timestamp_as_abs_t>::call(as_abstime(t), buf);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct timestamp_as_rel_t
	{
		time_t ts;
	};

	inline timestamp_as_rel_t as_reltime(time_t t)
	{
		timestamp_as_rel_t const r = { ts: t };
		return r;
	}

	template<>
	struct type_tunnel<timestamp_as_rel_t>
	{
		enum { buffer_size = sizeof("xxxxdays xx:xx:xx") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(timestamp_as_rel_t const& t, buffer_t const& buf = buffer_t())
		{
			static unsigned const seconds_in_hour = 60 * 60;
			static unsigned const seconds_in_day = seconds_in_hour * 24;

			unsigned days = t.ts / seconds_in_day;
			unsigned hours = (t.ts % seconds_in_day) / seconds_in_hour;
			unsigned mins = (t.ts % seconds_in_hour) / 60;
			unsigned secs = (t.ts % seconds_in_hour) % 60;

			ssize_t n = ::snprintf(buf.get(), buf.size(), "%04udays %02u:%02u:%02u", days % 10000, hours, mins, secs);

			BOOST_ASSERT(n == (buffer_size - 1));
			return buf.get();
		}
	};

	typedef type_tunnel<timestamp_as_rel_t>::buffer_t format_reltime_b_t;
	inline str_ref format_reltime(time_t t, format_reltime_b_t const& buf = format_reltime_b_t())
	{
		return type_tunnel<timestamp_as_rel_t>::call(as_reltime(t), buf);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__OS_TIMEVAL_HPP_

