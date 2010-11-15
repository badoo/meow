////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_
#define MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_

#include <cctype> 	// isdigit et. al.
#include <cstring> 	// memchr

#include <boost/bind.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <meow/mapping/http_headers.hpp>
#include <meow/mapping/kv_mapping.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////
// mapping object, that defines how headers are mapped into the structure

	template<class ContextT>
	struct http_header_mapping_t : public kv_mapping_t<ContextT>
	{
		typedef http_header_mapping_t 				self_t;
		typedef typename self_t::base_t 			base_t;

	public: // setters for setting up the mapping

		template<class Function>
		self_t& on_header(str_ref name, Function const& function)
		{
			base_t::on_name(name, function);
			return *this;
		}

		template<class Function>
		self_t& on_any_header(Function const& function)
		{
			base_t::on_any_name(function);
			return *this;
		}

		template<class ContainerT>
		self_t& put_all_headers_to(ContainerT ContextT::* cont)
		{
			this->on_any_header(seq_append(cont));
			return *this;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	// AnToXa: 
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
	do {									\
		unsigned char d = (c) ^ 0x30; 		\
		if (d < 10) 						\
			val = d; 						\
		else 								\
			return false; 					\
	} while(0)								\
/**/


////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	inline bool map_http_request_line(CharT *& head, CharT * const hend, http_request_line_t& result)
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
		while (head != hend && detail::my_isspace(*head)) ++head;
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
		while (detail::my_isspace(*--vstr));
		result.uri.assign(head, vstr + 1);

		head = lf;
		return true;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	inline bool map_http_response_line(CharT *& head, CharT * const hend, http_response_line_t& result)
	{
		// requet has the form
		// HTTP/<number>.<number><+space><number><+space><random_message>\r\n

		BOOST_STATIC_ASSERT(1 == sizeof(CharT));

		// prefix
		static str_ref const prefix = ref_lit("HTTP/");
		static size_t const prefix_len = sizeof("HTTP/x.x") - 1;

		// check full length right at start
		BOOST_ASSERT(head < hend);
		if (__builtin_expect(size_t(hend - head) < prefix_len, 0))
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
		while (head != hend && detail::my_isspace(*head)) ++head;
		if (__builtin_expect(head == hend, 0)) { return false; }

		// response code
		head = detail::read_number<4>(head, hend, result.status);
		if (__builtin_expect(head == hend, 0)) { return false; }

		// skip spaces
		while (head != hend && detail::my_isspace(*head)) ++head;
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

////////////////////////////////////////////////////////////////////////////////////////////////
// header parsers

	template<class CharT, class Function>
	inline bool map_http_headers(CharT *& head, CharT * const hend, Function const& header_fn)
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
			while (head != hend && detail::my_isspace(*head)) ++head;

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

#undef READ_DIGIT_OR_RETURN_FALSE

	template<class StringT, class Function>
	inline bool map_http_headers(StringT const& headers, Function const& header_fn)
	{
		typename StringT::iterator b = headers.begin();
		return map_http_headers(b, headers.end(), header_fn);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// full request/response parsers

	template<class CharT, class MappingT, class ContextT>
	inline bool map_http_request(CharT *& head, CharT * const hend, MappingT const& m, ContextT *ctx)
	{
		return map_http_request_line(head, hend, *ctx)
			&& map_http_headers(head, hend, boost::bind(kv_mapping_executor_t(), &m, ctx, _1, _2))
			;
	}

	template<class StringT, class MappingT, class ContextT>
	inline bool map_http_request(StringT const& str, MappingT const& m, ContextT *ctx)
	{
		typename StringT::iterator b = str.begin();
		return map_http_request(b, str.end(), m, ctx);
	}

	template<class CharT, class MappingT, class ContextT>
	inline bool map_http_response(CharT *& head, CharT * const hend, MappingT const& m, ContextT *ctx)
	{
		return map_http_response_line(head, hend, *ctx)
			&& map_http_headers(head, hend, boost::bind(kv_mapping_executor_t(), &m, ctx, _1, _2))
			;
	}

	template<class StringT, class MappingT, class ContextT>
	inline bool map_http_response(StringT const& str, MappingT const& m, ContextT *ctx)
	{
		typename StringT::iterator b = str.begin();
		return map_http_response(b, str.end(), m, ctx);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_

