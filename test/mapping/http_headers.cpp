////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

// cd meow/test/mapping
// g++ -O0 -g3 -o http_headers -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 http_headers.cpp
//

#include <cstring> // memchr

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/str_ref.hpp>
//#include <meow/str_ref_algo.hpp>

using namespace meow;
namespace ff = meow::format;

#include <meow/mapping/http_headers.hpp>

using namespace meow::mapping;

////////////////////////////////////////////////////////////////////////////////////////////////

// antoxa: 
// no idea, but having this function instead of directly calling isdigit()
//  speeds stuff up by around 10% on my core2duo 2.8gz
inline bool my_isdigit(unsigned char const c)
{
	return std::isdigit(c);//(c ^ 0x30) < 10;
}

inline bool my_isspace(unsigned char const c)
{
	return std::isspace(c);
}

template<size_t max_len, class CharT, class T>
inline CharT* read_number(CharT *head, CharT *hend, T& v)
{
	BOOST_STATIC_ASSERT(max_len >= 1);

	if (__builtin_expect(hend < head + max_len, 0))
		hend = head + max_len;

	if (__builtin_expect(my_isdigit(*head), 1))
	{
		v = *head++ - '0';

		for (; head != hend && my_isdigit(*head); ++head)
		{
			v *= 10;
			v += *head - '0';
		}

		return head;
	}
	else
	{
		v = 0;
		return hend;
	}
}

#define READ_DIGIT_OR_RETURN_FALSE(c, val) 	\
do {										\
	unsigned char d = (c) ^ 0x30; 			\
	if (d < 10) 							\
		val = d; 							\
	else 									\
		return false; 						\
} while(0)									\
/**/

////////////////////////////////////////////////////////////////////////////////////////////////

template<class CharT>
bool process_request_line(CharT *& head, CharT * const hend, http_request_line_t& result)
{
	// requet has the form
	// VERB<+space><random_crap_uri><+space>HTTP/<number>.<number>\r\n

	BOOST_STATIC_ASSERT(1 == sizeof(CharT));

	// find first space to get the VERB
	CharT *vend = (CharT*)std::memchr(head, ' ', hend - head);
	if (__builtin_expect(NULL == vend, 0))
		return false;

	result.method.assign(head, vend);
	head = vend + 1;

	// skip spaces if any, optimize for just 1 space
	while (head != hend && my_isspace(*head)) ++head;
	if (__builtin_expect(head == hend, 0)) { return false; }

	// now check for HTTP/n.n at the end
	//  everything in between will be url

	//  first: check for proper offsets here
	static str_ref const vprefix = ref_lit(" HTTP/");
	static size_t const vstr_len = sizeof(" HTTP/x.x") - 1;
	if (__builtin_expect((hend - head) < (vstr_len + 2 /*\r\n*/), 0))
		return false;

	// second: find \r\n
	CharT *lf = (CharT*)std::memchr(head, '\n', (hend - head));
	if (__builtin_expect(NULL != lf, 1))
	{
		if (__builtin_expect('\r' != *(lf - 1), 0))
			return false;

		++lf;
	}

	// third: parse on from here
	CharT *vstr = lf - vstr_len - 2 /* \r\n */;
	if (0 != std::memcmp(vstr, vprefix.data(), vprefix.size()))
		return false;

	// major ver
	READ_DIGIT_OR_RETURN_FALSE(*(lf - 5), result.version.major);

	// sep
	if (__builtin_expect('.' != *(lf - 4), 0))
		return false;

	// minor ver
	READ_DIGIT_OR_RETURN_FALSE(*(lf - 3), result.version.minor);

	// uri finally, skip spaces at the end
	while (my_isspace(*--vstr));
	result.uri.assign(head, vstr + 1);

	head = lf;
	return true;
}

template<class CharT>
bool process_response_line(CharT *& head, CharT * const hend, http_response_line_t& result)
{
	// requet has the form
	// HTTP/<number>.<number><+space><number><+space><random_message>\r\n

	BOOST_STATIC_ASSERT(1 == sizeof(CharT));

	// prefix
	static str_ref const prefix = ref_lit("HTTP/");
	static size_t const prefix_len = sizeof("HTTP/x.x") - 1;

	// check full length right at start
	if (__builtin_expect((hend - head) < prefix_len, 0))
		return false;

	if (__builtin_expect(0 != std::memcmp(head, prefix.data(), prefix.size()), 0))
		return false;

	head += prefix_len;

	// read major version number
	READ_DIGIT_OR_RETURN_FALSE(*(head - 3), result.version.major);

	// ver separator
	if (__builtin_expect('.' != *(head - 2), 0))
		return false;

	// read minor version number
	READ_DIGIT_OR_RETURN_FALSE(*(head - 1), result.version.minor);

	// skip spaces
	while (head != hend && my_isspace(*head)) ++head;
	if (__builtin_expect(head == hend, 0)) { return false; }

	// response code
	head = read_number<4>(head, hend, result.status);
	if (__builtin_expect(head == hend, 0)) { return false; }

	// skip spaces
	while (head != hend && my_isspace(*head)) ++head;
	if (__builtin_expect(head == hend, 0)) { return false; }

	// find where the line ends
	if (__builtin_expect((hend - head) < 2, 0))
		return false;

	CharT *lf = (CharT*)std::memchr(head, '\n', (hend - head));
	if (__builtin_expect(NULL == lf, 0))
	{
		result.message.assign(head, hend);
		head = hend;
		return true;
	}

	if (__builtin_expect('\r' != *--lf, 0))
		return false;

	result.message.assign(head, lf);
	head = lf + 2;
	return true;
}

template<class CharT, class Function>
bool process_headers(CharT *& head, CharT * const hend, Function const& header_fn)
{
	BOOST_STATIC_ASSERT(1 == sizeof(CharT));

	while (head != hend)
	{
		CharT const *col = (CharT*)memchr(head, ':', hend - head);
		// not found: either error or end of headers
		if (__builtin_expect(NULL == col, 0))
			break;

		// got header name
		str_ref const name = str_ref(head, col);

		head = col + 1;

		// skip whitespace
		while (head != hend && my_isspace(*head)) ++head;

		// value is everything to \r\n
		if (__builtin_expect((hend - head) < 2, 0))
			return false;

		CharT const *lf = (CharT*)std::memchr(head, '\n', hend - head);
		if (__builtin_expect(NULL != lf, 1))
		{
			if (__builtin_expect('\r' != *--lf, 0))
				return false;
		}

		str_ref const value = str_ref(head, lf);
		header_fn(name, value);

		head = lf + 2;
	}

	return true;
}

#include <boost/bind.hpp>
#include <meow/mapping/kv_mapping.hpp>

template<class MappingT, class ContextT>
void map_headers_new(str_ref const headers, MappingT const& m, ContextT *ctx)
{
	char const *b = headers.begin();
	process_headers(b, headers.end(), boost::bind(kv_mapping_executor_t(), &m, ctx, _1, _2));
}

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
	bool done = process_request_line(b, req_line.end(), l);

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
	bool done = process_response_line(b, resp_line.end(), l);

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
	bool done = process_headers(b, headers.end(), f);
	ff::fmt(stdout, "done: {0}\n", done);
	ff::fmt(stdout, "b: '{0}'\n", b);
}

//#include <meow/mapping/http_headers_impl.hpp>

int main(int argc, char **argv)
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

	size_t n_iterations = 10 * 1000 * 1000;

	if (argc >= 2)
		n_iterations = atoi(argv[1]);

	ff::fmt(stdout, "n_iterations: {0}\n", n_iterations);

	if (argc>= 3)
	{
		if (ref_lit("h") == argv[2])
		{
			kv_mapping_t<str_ref> m;
			m.on_name("Connection", &do_stuff);
/*
			http_header_mapping_t<str_ref> m_2;
			m_2.on_header("Connection", &do_stuff);
*/
			for (size_t i = 0; i < n_iterations; ++i)
			{
				str_ref ctx;
				//map_headers_new(headers, m_2, &ctx);
				map_headers_new(headers, m, &ctx);
/*
				char const *b = headers.begin();
				process_headers(b, headers.end(), &do_nothing);
*/
			}
		}
		if (ref_lit("l") == argv[2])
		{
/*
			for (size_t i = 0; i < n_iterations; ++i)
			{
				http_request_line_t ln;
				str_ref::iterator b = req_line.begin();
				map_http_request_line(b, req_line.end(), ln);
			}
*/
		}
		if (ref_lit("my_l") == argv[2])
		{

			for (size_t i = 0; i < n_iterations; ++i)
			{
				http_request_line_t l;
				char const *b = req_line.begin();
				process_request_line(b, req_line.end(), l);
			}
		}
		if (ref_lit("r") == argv[2])
		{
/*
			for (size_t i = 0; i < n_iterations; ++i)
			{
				http_request_line_t ln;
				str_ref::iterator b = resp_line.begin();
				map_http_request_line(b, resp_line.end(), ln);
			}
*/
		}
		if (ref_lit("my_r") == argv[2])
		{
			for (size_t i = 0; i < n_iterations; ++i)
			{
				http_response_line_t l;
				char const *b = resp_line.begin();
				bool done = process_response_line(b, resp_line.end(), l);
			}
		}
	}
	else
	{
#if 0
//		kv_mapping_t<str_ref> m;
//		m.on_name("Connection", &do_stuff);

		http_header_mapping_t<str_ref> m;
		m.on_header("Connection", &do_stuff);

		using namespace boost::spirit;

		for (size_t i = 0; i < n_iterations; ++i)
		{
			str_ref ctx;
			str_ref::iterator b = headers.begin();
///*
			map_http_headers(b, headers.end(), m, ctx);
//*/
//			str_ref ctx;
//			m.call_handler_impl(ctx, ref_lit("a"), ref_lit("b"));
/*
			static meow::mapping::detail::http_headers_grammar_t<char const*, str_ref> grammar_;
			boost::spirit::qi::parse(b, headers.end(), grammar_, ctx);
*/
		}
#endif
	}

	return 0;
}

