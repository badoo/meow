////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

// cd meow/test/mapping
// g++ -O0 -g3 -o http_url -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 http_url.cpp
//

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/mapping/http_url.hpp>

using meow::str_ref;
using meow::ref_lit;
using namespace meow::mapping;
namespace ff = meow::format;

////////////////////////////////////////////////////////////////////////////////////////////////

void dump_url(http_url_t const& u)
{
	ff::fmt(stdout,
			"{{\n"
			"  scheme: {0}\n"
			"  host: {1}\n"
			"  port: {2}\n"
			"  path: {3}\n"
			"  args: {4}\n"
			"  anchor: {5}\n"
			"}\n"
			, u.scheme
			, u.host
			, u.port
			, u.path
			, u.args
			, u.anchor
			);
}

void test_url(str_ref url)
{
	http_url_t u;
	char const *b = url.begin();
	bool done = map_http_url(b, url.end(), &u);

	ff::fmt(stdout, "done: {0}\n", done);
	dump_url(u);
}

void test_url_path(str_ref url)
{
	http_url_t u;
	char const *b = url.begin();
	bool done = map_http_url_path(b, url.end(), &u);

	ff::fmt(stdout, "done: {0}\n", done);
	dump_url(u);
}

////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	test_url("http://badoo.com");
	test_url("http://p62.badoo.com:8000/151/7/5/5/34/1026399/1027967_300.jpg?updated=1272028232");
	test_url("http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#COMPILER_WARNINGS");
	test_url("http://nvie.com/posts/a-successful-git-branching-model/");
	test_url("http://www-cs-students.stanford.edu/~blynn/gitmagic/apa.html#_sha1_weaknesses");
	test_url("http://www.google.ru/search?hl=en&newwindow=1&client=opera&hs=XQ2&rls=en&q=ragel&aq=f&aqi=g2g-s1g2g-s1g2g-s1g1&aql=&oq=&gs_rfai=");
	test_url("http://badoo.com:98//?lalala=asbs#mama");

	test_url_path("/151/7/5/5/34/1026399/1027967_300.jpg?updated=1272028232");
}

