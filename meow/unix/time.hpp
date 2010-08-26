////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__TIME_HPP_
#define MEOW_UNIX__TIME_HPP_

#include <sys/types.h>
#include <sys/time.h>			// for gettimeofday, settimeofday, timeval, etc.

#include <cmath>				// for modf
#include <ctime>

#include <boost/assert.hpp>

#include <meow/api_call_error.hpp>
#include <meow/tmp_buffer.hpp>

#include <meow/unix/libc_wrapper.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct timeval os_timeval_t;
	typedef struct timezone os_timezone_t;

	enum
	{
		  msec_in_sec = 1000
		, usec_in_sec = 1000 * 1000
		, nsec_in_sec = 1000 * 1000 * 1000
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	inline double os_timeval_to_double(os_timeval_t const& tv)
	{
		double r;
		r = tv.tv_usec;
		r /= usec_in_sec;
		r += tv.tv_sec;
		return r;
	}

	inline os_timeval_t os_timeval_from_double(double const d)
	{
		BOOST_ASSERT(d >= 0.0);

		double sec_d = 0.0;
		double usec_d = modf(d, &sec_d);

		os_timeval_t const tv = {
			  tv_sec: static_cast<time_t>(sec_d)
			, tv_usec: static_cast<suseconds_t>(usec_d * usec_in_sec)
		};
		return tv;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool operator==(os_timeval_t const& lhs, os_timeval_t const& rhs)
	{
		return (lhs.tv_sec == rhs.tv_sec) && (lhs.tv_usec == rhs.tv_usec);
	}

	inline bool operator!=(os_timeval_t const& lhs, os_timeval_t const& rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator<(os_timeval_t const& l, os_timeval_t const& r)
	{
		if (l.tv_sec < r.tv_sec)
			return true;
		if (l.tv_sec > r.tv_sec)
			return false;
		return l.tv_usec < r.tv_usec;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline os_timeval_t& operator-=(os_timeval_t& lhs, os_timeval_t const& rhs)
	{
		lhs.tv_sec -= rhs.tv_sec;
		lhs.tv_usec -= rhs.tv_usec;

		if (lhs.tv_usec < 0)
		{
			lhs.tv_sec -= 1;
			lhs.tv_usec += usec_in_sec;
		}

		return lhs;
	}

	inline os_timeval_t& operator+=(os_timeval_t& lhs, os_timeval_t const& rhs)
	{
		lhs.tv_sec += rhs.tv_sec;
		lhs.tv_usec += rhs.tv_usec;

		if (lhs.tv_usec > usec_in_sec)
		{
			lhs.tv_sec += 1;
			lhs.tv_usec -= usec_in_sec;
		}
		
		return lhs;
	}

	inline os_timeval_t operator-(os_timeval_t lhs, os_timeval_t const& rhs) { return lhs -= rhs; }
	inline os_timeval_t operator+(os_timeval_t lhs, os_timeval_t const& rhs) { return lhs += rhs; }

////////////////////////////////////////////////////////////////////////////////////////////////

	namespace time_detail_ {

		typedef meow::tmp_buffer<sizeof("-1234567890.123456")> tv_buf_t;

	} // namespace detail
	
	inline char const* os_timeval_tmp_str(timeval const& tv, time_detail_::tv_buf_t const& buf = time_detail_::tv_buf_t())
	{
		size_t n = ::snprintf(buf.get(), buf.size(), "%ld.%.6d", tv.tv_sec, static_cast<int>(tv.tv_usec));
		BOOST_ASSERT(n <= (time_detail_::tv_buf_t::buffer_size - 1));
		return buf.get();
	}

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

	inline time_t time_ex()
	{
		return ::time(NULL);
	}

	inline os_timeval_t gettimeofday_ex()
	{
		os_timeval_t tv;
		gettimeofday_ex(&tv, NULL /* timezone ptr must be always null */);
		return tv;
	}

	inline void settimeofday_ex(os_timeval_t const& tv)
	{
		settimeofday_ex(&tv, NULL /* timezone ptr must be always null */);
	}

	inline os_timeval_t make_timeval(time_t sec, suseconds_t usec)
	{
		BOOST_ASSERT(usec < usec_in_sec);
		os_timeval_t tv = { tv_sec: sec, tv_usec: usec };
		return tv;
	}

	inline os_timeval_t null_timeval() { return make_timeval(0, 0); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__TIME_HPP_

