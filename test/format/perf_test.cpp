////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/format/
// g++ -O0 -g3 -o perf_test -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 perf_test.cpp
//

#include <meow/format/sink/FILE.hpp>
#include <meow/format/sink/char_buffer.hpp>

#include <meow/format/inserter/floating_point.hpp>
#include <meow/format/inserter/integral.hpp>
#include <meow/format/inserter/pointer.hpp>
#include <meow/format/inserter/hex_string.hpp>
#include <meow/format/inserter/timeval.hpp>

#include <meow/format/format.hpp>

#include <meow/unix/ipv4_address.hpp>
#include <meow/unix/socket.hpp>
#include <meow/unix/time.hpp>

namespace ff = meow::format;
using ff::fmt;
using ff::write;

using meow::str_ref;
using meow::ref_lit;

int main()
{
	char buf[1024];
	size_t const n_iterations = 1 * 1000 * 1000;

	int const number = 1234567890;
	size_t const sz_number = 34734893849384000;
	os_timeval_t const tv = os_unix::gettimeofday_ex();

	for (size_t i = 0; i < n_iterations; ++i)
	{
		ff::sink::char_buffer_sink_t sink(buf, sizeof(buf));
//		fmt(sink, "floating point test, number: {0}, as_hex: {1}\n", number, ff::as_hex(number));

		ff::fmt(sink
				, "POST {0} HTTP/1.0\r\n"
				  "Host: {1}\r\n"
				  "Connection: close\r\n"
				  "Content-type: application/octet-stream\r\n"
				  "Content-length: {2}\r\n"
				  "User-Agent: {3}\r\n"
				  "{4}: {5}\r\n"
				, ref_lit("/some_url?lala=aaaa")
				, ref_lit("wwwbma.mlan")
				, tv
				, ref_lit("bma-proxy/1.0.5")
				, ref_lit("X-Client-Ip"), ff::addr_as_ip(os_sockaddr_in_t())
			);

/*
		str_ref slices[40];
		str_ref arg_slices[6] = {
				  ref_lit("/some_url?lala=aaaa")
				, ref_lit("wwwbma.mlan")
				, ff::type_tunnel<os_timeval_t>::call(tv)
				, ref_lit("bma-proxy/1.0.5")
				, ref_lit("X-Client-Ip"), "0.0.0.0"//ff::type_tunnel<ff::ipv4_address_just_ip_t>::call(ff::addr_as_ip(os_sockaddr_in_t()))
				};

		ff::format_info_t fi = ff::parse_format_expression("POST {0} HTTP/1.0\r\n"
				  "Host: {1}\r\n"
				  "Connection: close\r\n"
				  "Content-type: application/octet-stream\r\n"
				  "Content-length: {2}\r\n"
				  "User-Agent: {3}\r\n"
				  "{4}: {5}\r\n"
				  , slices, 40
				  , arg_slices, 6
				  );
*/
/*
		ff::write(sink
				, "POST ", ref_lit("/some_url?lala=aaaa"), " HTTP/1.0\r\n"
				  "Host: ", ref_lit("wwwbma.mlan"), "\r\n"
				  "Connection: close\r\n"
				  "Content-type: application/octet-stream\r\n"
				  "Content-length: ", tv, "\r\n"
				  "User-Agent: ", ref_lit("bma-proxy/1.0.5"), "\r\n"
				, ref_lit("X-Client-Ip"), ": ", ff::addr_as_ip(os_sockaddr_in_t()), "\r\n"
			);
*/
	}

	return 0;
}


