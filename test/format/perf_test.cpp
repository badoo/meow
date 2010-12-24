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

#include <meow/format/format.hpp>

#include <meow/unix/ipv4_address.hpp>
#include <meow/unix/socket.hpp>

namespace ff = meow::format;
using ff::fmt;
using ff::write;

using meow::ref_lit;

int main()
{
	char buf[1024];
	size_t const n_iterations = 1 * 1000 * 1000;

	int number = 1234567890;

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
				, size_t(23434)
				, ref_lit("bma-proxy/1.0.5")
				, ref_lit("X-Client-Ip"), ff::addr_as_ip(os_sockaddr_in_t())
			);

	}

	return 0;
}

