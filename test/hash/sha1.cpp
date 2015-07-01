////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

// cd meow/test/hash/
// g++ -O3 -mtune=generic -g3 -o sha1 -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 sha1.cpp

#include <meow/hash/sha1.hpp>

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/stopwatch.hpp>

namespace ff = meow::format;
using namespace meow;

void print_test(str_ref str, str_ref digest_check)
{
	ff::fmt(stdout, "{0} <-- computed\n{1} <-- test\n\n", sha1_digest(str), digest_check);
}

int main()
{
	print_test("test", "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3");
	print_test("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
	print_test("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1");

	static size_t const n_iterations = 1 * 1000 * 1000;

	meow::str_ref s = "medvedalsndlaksnd;am000";

	stopwatch_t sw;

	for (size_t i = 0; i < n_iterations; ++i)
	{
		sha1_digest_t d = sha1_digest(s);
	}

	double const diff_tv = timeval_to_double(sw.stamp());

	ff::fmt(stdout, "{0} iterations, {1}s\n~{2} hashes/sec\n"
			, n_iterations, diff_tv
			, ff::float_fmt("%.2f", n_iterations / diff_tv)
			);

	return 0;
}

