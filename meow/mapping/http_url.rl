%%{
	machine http_url;

	# vim: set syntax=ragel autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
	# (c) 2010 Anton Povarov <anton.povarov@gmail.com>
}%%

////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

// WARNING: generated from .rl file nearby (which is a ragel 'program')
// command: ragel -G2 -o mapping/http_url.hpp mapping/http_url.rl
// see: http://www.complang.org/ragel/ for more info

#ifndef MEOW_MAPPING__HTTP_URL_HPP_
#define MEOW_MAPPING__HTTP_URL_HPP_

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct http_url_t
	{
		str_ref scheme;
		str_ref host;
		str_ref port;
		str_ref path;
		str_ref args;
		str_ref anchor;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	%%{
		action set_begin {
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}

		action set_scheme {
			result->scheme.assign(begin, p);
	//		ff::fmt(stdout, "set_scheme\n");
		}

		action set_host {
			result->host.assign(begin, p);
	//		ff::fmt(stdout, "set_host\n");
		}

		action set_port {
			result->port.assign(begin, p);
	//		ff::fmt(stdout, "set_port\n");
		}

		action set_path {
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}

		action set_args {
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}

		action set_anchor {
			result->anchor.assign(begin, p);
	//		ff::fmt(stdout, "set_anchor\n");
		}

		scheme = [a-z0-9\-]+ >set_begin %set_scheme;
		host_part = [a-z0-9\-]+;
		host = (host_part ('\.' host_part)*) >set_begin %set_host;
		port = ':' digit{1,5} >set_begin %set_port;
		anchor = ('#' any*) >set_begin %set_anchor;
		args = '?' ((any - [#])* >set_begin %set_args);
		path = (('/' (any - [?#])*)+ >set_begin %set_path) args? anchor?;

		just_path := path;
		main := scheme "://" host port? path?;
	}%%

////////////////////////////////////////////////////////////////////////////////////////////////
namespace http_url_ {
////////////////////////////////////////////////////////////////////////////////////////////////

	%%write data;

	template<int initial_state, class CharT, class ResultT>
	inline bool parse(CharT *& b, CharT *e, ResultT *result)
	{
		char const *p = b;
		char const *pe = e;
		char const *eof = pe;
		char const *begin = NULL;

		int cs = initial_state;
		%% write exec;

		b = p;
		return (cs >= http_url_::http_url_first_final);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace http_url_ {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool map_http_url_path(char const *& b, char const *e, http_url_t *result)
	{
		return http_url_::parse<http_url_::http_url_en_just_path>(b, e, result);
	}

	inline bool map_http_url(char const *& b, char const *e, http_url_t *result)
	{
		return http_url_::parse<http_url_::http_url_en_main>(b, e, result);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__HTTP_URL_HPP_

