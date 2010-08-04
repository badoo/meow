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
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__OS_TIMEVAL_HPP_

