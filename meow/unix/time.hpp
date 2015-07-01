////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__TIME_HPP_
#define MEOW_UNIX__TIME_HPP_

#include <sys/types.h>
#include <sys/time.h>			// for gettimeofday, settimeofday, timeval, etc.

#include <cassert>
#include <cmath>				// for modf
#include <ctime>

#include <meow/api_call_error.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/unix/libc_wrapper.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct timeval os_timeval_t;
	typedef struct timezone os_timezone_t;
	typedef struct timespec os_timespec_t;

	struct timeval_t
	{
		time_t tv_sec;
		long   tv_nsec; // nanoseconds
	};

	struct duration_t
	{
		int64_t nsec;   // nanoseconds
	};

	enum
	{
		  msec_in_sec = 1000
		, usec_in_sec = 1000 * 1000
		, nsec_in_sec = 1000 * 1000 * 1000
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	inline double timeval_to_double(timeval_t const& tv)
	{
		double r;
		r = tv.tv_nsec;
		r /= nsec_in_sec;
		r += tv.tv_sec;
		return r;
	}

	inline timeval_t timeval_from_double(double const d)
	{
		assert(d >= 0.0);

		double sec_d = 0.0;
		double const nsec_d = modf(d, &sec_d);

		timeval_t const tv = {
			.tv_sec = static_cast<time_t>(sec_d),
			.tv_nsec = static_cast<long>(nsec_d * nsec_in_sec)
		};
		return tv;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool operator==(timeval_t const& lhs, timeval_t const& rhs)
	{
		return (lhs.tv_sec == rhs.tv_sec) && (lhs.tv_nsec == rhs.tv_nsec);
	}

	inline bool operator!=(timeval_t const& lhs, timeval_t const& rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator<(timeval_t const& lhs, timeval_t const& rhs)
	{
		if (lhs.tv_sec < rhs.tv_sec)
			return true;
		if (lhs.tv_sec > rhs.tv_sec)
			return false;
		return lhs.tv_nsec < rhs.tv_nsec;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline timeval_t& operator-=(timeval_t& lhs, timeval_t const& rhs)
	{
		lhs.tv_sec -= rhs.tv_sec;
		lhs.tv_nsec -= rhs.tv_nsec;

		if (lhs.tv_nsec < 0)
		{
			lhs.tv_sec -= 1;
			lhs.tv_nsec += nsec_in_sec;
		}

		return lhs;
	}

	inline timeval_t& operator+=(timeval_t& lhs, timeval_t const& rhs)
	{
		lhs.tv_sec += rhs.tv_sec;
		lhs.tv_nsec += rhs.tv_nsec;

		if (lhs.tv_nsec > nsec_in_sec)
		{
			lhs.tv_sec += 1;
			lhs.tv_nsec -= nsec_in_sec;
		}
		
		return lhs;
	}

	inline timeval_t operator-(timeval_t lhs, timeval_t const& rhs) { return lhs -= rhs; }
	inline timeval_t operator+(timeval_t lhs, timeval_t const& rhs) { return lhs += rhs; }

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, gettimeofday
											, ((os_timeval_t*, "%p"))
											  ((os_timezone_t*, "%p"))
											);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, settimeofday
											, ((os_timeval_t const*, "%p"))
											  ((os_timezone_t const*, "%p"))
											);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, clock_gettime
											, ((clockid_t, "%d"))
											  ((os_timespec_t*, "%p"))
											);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, clock_settime
											, ((clockid_t, "%d"))
											  ((os_timespec_t const*, "%p"))
											);

	inline time_t time_ex()
	{
		return ::time(NULL);
	}

	inline timeval_t gettimeofday_ex()
	{
		os_timeval_t tv;
		gettimeofday_ex(&tv, NULL /* timezone ptr must be always null */);

		timeval_t const result = {
			.tv_sec  = tv.tv_sec,
			.tv_nsec = tv.tv_usec * (nsec_in_sec / usec_in_sec),
		};
		return result;
	}

	inline timeval_t clock_gettime_ex(clockid_t clk_id)
	{
		os_timespec_t ts;
		int r = clock_gettime(clk_id, &ts);

		if (__builtin_expect((r != 0), 0))
			return {};

		timeval_t const result = {
			.tv_sec  = ts.tv_sec,
			.tv_nsec = ts.tv_nsec,
		};
		return result;
	}

	inline timeval_t clock_monotonic_now()
	{
		return clock_gettime_ex(CLOCK_MONOTONIC);
	}

#if 0
	// deprecated, because we've changed from os_timeval_t to timeval_t
	//
	inline os_timeval_t make_timeval(time_t sec, suseconds_t usec)
	{
		assert(usec < usec_in_sec);
		os_timeval_t tv = { .tv_sec = sec, .tv_usec = usec };
		return tv;
	}

	inline os_timeval_t null_timeval() { return make_timeval(0, 0); }
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__TIME_HPP_

