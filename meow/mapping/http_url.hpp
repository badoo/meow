
#line 1 "mapping/http_url.rl"

#line 6 "mapping/http_url.rl"


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

	
#line 84 "mapping/http_url.rl"


////////////////////////////////////////////////////////////////////////////////////////////////
namespace http_url_ {
////////////////////////////////////////////////////////////////////////////////////////////////

	
#line 47 "mapping/http_url.hpp"
static const int http_url_start = 1;
static const int http_url_first_final = 9;
static const int http_url_error = 0;

static const int http_url_en_just_path = 8;
static const int http_url_en_main = 1;


#line 91 "mapping/http_url.rl"

	template<int initial_state, class CharT, class ResultT>
	inline bool parse(CharT *& b, CharT *e, ResultT *result)
	{
		char const *p = b;
		char const *pe = e;
		char const *eof = pe;
		char const *begin = NULL;

		int cs = initial_state;
		
#line 68 "mapping/http_url.hpp"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 45 )
		goto tr0;
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr0;
	} else if ( (*p) >= 48 )
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 97 "mapping/http_url.hpp"
	switch( (*p) ) {
		case 45: goto st2;
		case 58: goto tr3;
	}
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st2;
	} else if ( (*p) >= 48 )
		goto st2;
	goto st0;
tr3:
#line 44 "mapping/http_url.rl"
	{
			result->scheme.assign(begin, p);
	//		ff::fmt(stdout, "set_scheme\n");
		}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 119 "mapping/http_url.hpp"
	if ( (*p) == 47 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 47 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 45 )
		goto tr6;
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr6;
	} else if ( (*p) >= 48 )
		goto tr6;
	goto st0;
tr6:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 153 "mapping/http_url.hpp"
	switch( (*p) ) {
		case 46: goto st6;
		case 47: goto tr11;
		case 58: goto tr12;
	}
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st9;
	} else if ( (*p) >= 45 )
		goto st9;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 45 )
		goto st9;
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st9;
	} else if ( (*p) >= 48 )
		goto st9;
	goto st0;
tr11:
#line 49 "mapping/http_url.rl"
	{
			result->host.assign(begin, p);
	//		ff::fmt(stdout, "set_host\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st10;
tr21:
#line 54 "mapping/http_url.rl"
	{
			result->port.assign(begin, p);
	//		ff::fmt(stdout, "set_port\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 205 "mapping/http_url.hpp"
	switch( (*p) ) {
		case 35: goto tr14;
		case 63: goto tr15;
	}
	goto st10;
tr14:
#line 59 "mapping/http_url.rl"
	{
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st11;
tr18:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
	goto st11;
tr20:
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 251 "mapping/http_url.hpp"
	goto st11;
tr15:
#line 59 "mapping/http_url.rl"
	{
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 264 "mapping/http_url.hpp"
	if ( (*p) == 35 )
		goto tr18;
	goto tr17;
tr17:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 279 "mapping/http_url.hpp"
	if ( (*p) == 35 )
		goto tr20;
	goto st13;
tr12:
#line 49 "mapping/http_url.rl"
	{
			result->host.assign(begin, p);
	//		ff::fmt(stdout, "set_host\n");
		}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 294 "mapping/http_url.hpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr8;
	goto st0;
tr8:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 309 "mapping/http_url.hpp"
	if ( (*p) == 47 )
		goto tr21;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 47 )
		goto tr21;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st16;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 47 )
		goto tr21;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 47 )
		goto tr21;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st18;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 47 )
		goto tr21;
	goto st0;
case 8:
	if ( (*p) == 47 )
		goto tr9;
	goto st0;
tr9:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 364 "mapping/http_url.hpp"
	switch( (*p) ) {
		case 35: goto tr27;
		case 63: goto tr28;
	}
	goto st19;
tr27:
#line 59 "mapping/http_url.rl"
	{
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st20;
tr31:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
	goto st20;
tr33:
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 410 "mapping/http_url.hpp"
	goto st20;
tr28:
#line 59 "mapping/http_url.rl"
	{
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 423 "mapping/http_url.hpp"
	if ( (*p) == 35 )
		goto tr31;
	goto tr30;
tr30:
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 438 "mapping/http_url.hpp"
	if ( (*p) == 35 )
		goto tr33;
	goto st22;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 9: 
#line 49 "mapping/http_url.rl"
	{
			result->host.assign(begin, p);
	//		ff::fmt(stdout, "set_host\n");
		}
	break;
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
#line 54 "mapping/http_url.rl"
	{
			result->port.assign(begin, p);
	//		ff::fmt(stdout, "set_port\n");
		}
	break;
	case 10: 
	case 19: 
#line 59 "mapping/http_url.rl"
	{
			result->path.assign(begin, p);
	//		ff::fmt(stdout, "set_path\n");
		}
	break;
	case 13: 
	case 22: 
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
	break;
	case 11: 
	case 20: 
#line 69 "mapping/http_url.rl"
	{
			result->anchor.assign(begin, p);
	//		ff::fmt(stdout, "set_anchor\n");
		}
	break;
	case 12: 
	case 21: 
#line 39 "mapping/http_url.rl"
	{
			begin = p;
	//		ff::fmt(stdout, "set_begin\n");
		}
#line 64 "mapping/http_url.rl"
	{
			result->args.assign(begin, p);
	//		ff::fmt(stdout, "set_args\n");
		}
	break;
#line 523 "mapping/http_url.hpp"
	}
	}

	_out: {}
	}

#line 102 "mapping/http_url.rl"

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

