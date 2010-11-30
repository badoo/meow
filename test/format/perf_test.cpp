////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/format/
// g++ -O0 -g3 -o perf_test -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 perf_test.cpp
//

#include <meow/format/sink/char_buffer.hpp>

#include <meow/format/inserter/floating_point.hpp>
#include <meow/format/inserter/integral.hpp>
#include <meow/format/inserter/pointer.hpp>
#include <meow/format/inserter/hex_string.hpp>

#include <meow/format/format.hpp>

namespace ff = meow::format;
using ff::fmt;
using ff::write;

int main()
{
	char buf[1024];

	int number = 1234567890;
	double const f_number = -12312.9283283;

	size_t const n_iterations = 10 * 1000 * 100;

	for (size_t i = 0; i < n_iterations; ++i)
	{
		ff::sink::char_buffer_sink_t sink(buf, sizeof(buf));
		fmt(sink, "floating point test, float: {0}, double: {1}, ldouble: {2}\n", float(f_number), double(f_number), (long double)f_number);
	}

	return 0;
}

