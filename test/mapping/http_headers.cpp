////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2007+ Anton Povarov <anton.povarov@gmail.com>
// $Id$
////////////////////////////////////////////////////////////////////////////////////////////////

// cd meow/test/mapping
// g++ -O0 -g3 -o http_headers -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 http_headers.cpp
//

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/mapping/http_headers.hpp>
#include <meow/mapping/http_headers_impl.hpp>

using meow::str_ref;
using meow::ref_lit;
using namespace meow::mapping;
namespace ff = meow::format;

void do_stuff(str_ref& ctx, str_ref v)
{
	int a = 10;
	int *b = &a;

//	ff::write(stdout, v, "\n");
}

void do_nothing(str_ref k, str_ref v)
{
}

void do_print_header_kv(str_ref k, str_ref v)
{
	ff::fmt(stdout, "{0}: '{1}'\n", k, v);
}

void test_req_line(str_ref const req_line)
{
	http_request_line_t l;
	char const *b = req_line.begin();
	bool done = map_http_request_line(b, req_line.end(), l);

	ff::fmt(stdout, "line: '{0}', done: {1}\n", req_line, done);
	ff::fmt(stdout, "l.version: {0}.{1}\n", l.version.major, l.version.minor);
	ff::fmt(stdout, "l.method: '{0}', uri: '{1}'\n", l.method, l.uri);
	ff::fmt(stdout, "b: '{0}'\n", b);
	ff::fmt(stdout, "\n");
}

void test_resp_line(str_ref const resp_line)
{
	http_response_line_t l;
	char const *b = resp_line.begin();
	bool done = map_http_response_line(b, resp_line.end(), l);

	ff::fmt(stdout, "line: '{0}', done: {1}\n", resp_line, done);
	ff::fmt(stdout, "l.version: {0}.{1}\n", l.version.major, l.version.minor);
	ff::fmt(stdout, "l.status: {0}, message: '{1}'\n", l.status, l.message);
	ff::fmt(stdout, "b: '{0}'\n", b);
	ff::fmt(stdout, "\n");
}

template<class Function>
void test_process_headers(str_ref const& headers, Function const& f)
{
	char const *b = headers.begin();
	bool done = map_http_headers(b, headers.end(), f);
	ff::fmt(stdout, "done: {0}\n", done);
	ff::fmt(stdout, "b: '{0}'\n", b);
}

int main()
{
	str_ref const req_line = ref_lit("GET /preved?medved=lalala#auch HTTP/1.1\r\n");
	str_ref const resp_line = ref_lit("HTTP/1.1 404 Not Found\r\n");

	str_ref const headers = ref_lit(
							"Connection: close\r\n"
							"Content-Transfer-Encoding: chunked\r\n"
							"Cache-Control: private\r\n"
							"Expires: i dunno when\r\n"
							"\r\n"
							);

	test_req_line(req_line);
	test_resp_line(resp_line);
	test_process_headers(headers, &do_print_header_kv);

	return 0;
}

