////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__FORMAT_TIMEVAL_HPP_
#define MEOW_LOGGING__FORMAT_TIMEVAL_HPP_

#include <cstdio>
#include <ctime>

#include <boost/assert.hpp>

#include <meow/tmp_buffer.hpp>
#include <meow/format/metafunctions.hpp>
#include <meow/format/inserter/integral.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE:
// this header is *NOT* a duplicate for format/inserter/timeval.hpp
//  as we have very special timeval formatting for logging purposes

	struct time_as_log_insert_wrapper_t
	{
		struct timeval const& tv;
	};

	inline time_as_log_insert_wrapper_t as_log_ts(struct timeval const& tv)
	{
		time_as_log_insert_wrapper_t const r = { tv: tv };
		return r;
	}

	template<>
	struct type_tunnel<time_as_log_insert_wrapper_t>
	{
		static size_t const buffer_size = sizeof("YYYYmmdd hhmmss.xxxxxx");
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(time_as_log_insert_wrapper_t const& twrap, buffer_t const& buf = buffer_t())
		{
			struct tm tm;
			::gmtime_r(&twrap.tv.tv_sec, &tm);

			char *begin = buf.begin();
			char *p = buf.end();

			suseconds_t const microseconds = twrap.tv.tv_usec % 1000000;

			p = detail::integer_to_string(begin, p - begin, microseconds);

			int const field_size = 6;
			int const printed_size = (buf.end() - p);
			for (int i = 0; i < field_size - printed_size; ++i)
				*--p = '0';

			*--p = '.';
			p = detail::integer_to_string(begin, p - begin, tm.tm_sec);
			p = detail::integer_to_string(begin, p - begin, tm.tm_min);
			p = detail::integer_to_string(begin, p - begin, tm.tm_hour);
			*--p = ' ';
			p = detail::integer_to_string(begin, p - begin, tm.tm_mday);
			if (tm.tm_mday < 10)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_mon + 1);
			if (tm.tm_mon < 9)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_year + 1900);

			return str_ref(p, buf.end());
#if 0
			size_t n = snprintf(
							  buf.begin(), buf.size()
							, "%04u%02u%02u %02u%02u%02u.%06u"
							, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday
							, tm.tm_hour, tm.tm_min, tm.tm_sec
							, (unsigned)twrap.tv.tv_usec
							);
			BOOST_ASSERT((n == (buffer_size - 1)));

			return str_ref(buf.get(), n);
#endif
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__FORMAT_TIMEVAL_HPP_

