////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_
#define MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_

#include <map>
#include <vector>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_int.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_rule.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include "http_headers.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
// this must be in global namespace

	BOOST_FUSION_ADAPT_STRUCT(
			  meow::mapping::http_version_t
			, (unsigned int, major)
			  (unsigned int, minor)
			);

	BOOST_FUSION_ADAPT_STRUCT(
			  meow::mapping::http_request_line_t
			, (meow::str_ref, 					method)
			  (meow::str_ref, 					uri)
			  (meow::mapping::http_version_t, 	version)
			);

	BOOST_FUSION_ADAPT_STRUCT(
			  meow::mapping::http_response_line_t
			, (meow::mapping::http_version_t, 	version)
			  (meow::mapping::http_status_t, 	status)
			  (meow::str_ref, 					message)
			);

	BOOST_FUSION_ADAPT_STRUCT(
			  meow::mapping::http_header_kvref_t
			, (meow::str_ref, name)
			  (meow::str_ref, value)
			);

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace qi = boost::spirit::qi;

////////////////////////////////////////////////////////////////////////////////////////////////
// mapping object, that defines how headers are mapped into the structure

template<class ContextT>
	struct http_header_mapping_t : private boost::noncopyable
	{
		typedef http_header_mapping_t self_t;
		typedef boost::function<void(ContextT&, str_ref)> handler_t;
		typedef boost::function<void(ContextT&, http_header_kvref_t)> header_handler_t;

		typedef std::map<str_ref, handler_t> 	map_t;
		typedef std::vector<header_handler_t> 	hhv_t;

	public: // setters for setting up the mapping

		template<class Function>
		self_t& on_header(str_ref name, Function const& function)
		{
			map_[name] = function;
			return *this;
		}

		template<class ContainerT>
		self_t& put_all_headers_to(ContainerT ContextT::* cont)
		{
			this->on_any_header(seq_append(cont));
			return *this;
		}

		template<class Function>
		self_t& on_any_header(Function const& function)
		{
			any_.push_back(function);
			return *this;
		}

	public:

		void call_handlers(ContextT *ctx, http_header_kvref_t const& kv) const
		{
			BOOST_ASSERT(ctx);

	//		printf("%s; %.*s = %.*s\n", __func__, kv.name.c_length(), kv.name.data(), kv.value.c_length(), kv.value.data());

			handler_t const *handler_f = this->get_handler(kv.name);
			if (handler_f)
				(*handler_f)(*ctx, kv.value);

			for (typename hhv_t::const_iterator i = any_.begin(), i_end = any_.end(); i != i_end; ++i)
				(*i)(*ctx, kv);
		}

	private:

		handler_t const* get_handler(str_ref key) const
		{
			typename map_t::const_iterator i = map_.find(key);
			return (map_.end() == i)
				? NULL
				: &((*i).second)
				;
		}

	private:
		hhv_t 		any_;
		map_t 		map_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Iterator>
	struct http_grammar_base_t
	{
		qi::rule<Iterator, http_version_t()> 	r_version;
		qi::rule<Iterator, char()> 				r_separator;
		qi::rule<Iterator, char()> 				r_token;

		http_grammar_base_t()
		{
			using namespace qi;

			// HTTP version string: HTTP/<number>.<number>
			r_version %= lit("HTTP/") >> int_ >> omit['.'] >> int_;

			// HTTP separators taken from: http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2
			// "(" | ")" | "<" | ">" | "@"
			// | "," | ";" | ":" | "\" | <">
			// | "/" | "[" | "]" | "?" | "="
			// | "{" | "}" | SP | HT
			r_separator = char_(" \t()<>@,;:\\\"/[]?={}\r\n");

			// HTTP token, anything but a control or separator
			r_token = char_ - r_separator - cntrl;
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Iterator>
	struct http_request_line_grammar_t 
		: public qi::grammar<Iterator, http_request_line_t()>
		, public http_grammar_base_t<Iterator>
	{
		qi::rule<Iterator, http_request_line_t()> r;

		http_request_line_grammar_t()
			: http_request_line_grammar_t::base_type(r)
		{
			using namespace qi;

			r %=   omit[*space] >> raw[+this->r_token] 				// method
				>> omit[+space] >> raw[+(char_ - cntrl - space)] 	// uri
				>> omit[+space] >> this->r_version 					// version
				>> eol;
		}
	};

	template<class Iterator>
	struct http_response_line_grammar_t
		: public qi::grammar<Iterator, http_response_line_t()>
		, public http_grammar_base_t<Iterator>
	{
		qi::rule<Iterator, http_response_line_t()> r;

		http_response_line_grammar_t()
			: http_response_line_grammar_t::base_type(r)
		{
			using namespace qi;

			r %=   this->r_version 						// version
				>> omit[+space] >> int_ 				// response code
				>> omit[+space] >> raw[*(char_ - eol)] 	// message
				>> eol;
		}
	};

	template<class Iterator, class ContextT>
	struct http_headers_grammar_t
		: public qi::grammar<Iterator, void()>
		, public http_grammar_base_t<Iterator>
	{
		typedef http_headers_grammar_t 			self_t;
		typedef ContextT 						context_t;
		typedef http_header_mapping_t<ContextT> mapping_t;

		context_t 			*ctx_;
		mapping_t const 	*map_;

		qi::rule<Iterator, http_header_kvref_t()> r_line;
		qi::rule<Iterator, void()> r;

		http_headers_grammar_t()
			: http_headers_grammar_t::base_type(r)
			, ctx_(NULL)
			, map_(NULL)
		{
			using namespace qi;
			using boost::cref;
			using boost::ref;

			r_line  %= (raw[+this->r_token] >> omit[':' >> *space]
					>> raw[*(char_ - eol)])
					>> eol;

			r %= +(r_line[boost::bind(&mapping_t::call_handlers, cref(map_), ref(ctx_), ::_1)]);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	// first line parsers
	template<class Iterator>
	bool map_http_request_line(Iterator& begin, Iterator const end, http_request_line_t& ctx)
	{
		static detail::http_request_line_grammar_t<Iterator> grammar_;
		return qi::parse(begin, end, grammar_, ctx);
	}

	template<class Iterator>
	bool map_http_response_line(Iterator& begin, Iterator const end, http_response_line_t& ctx)
	{
		static detail::http_response_line_grammar_t<Iterator> grammar_;
		return qi::parse(begin, end, grammar_, ctx);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// header parsers

	template<class Iterator, class ContextT>
	bool map_http_headers(
			  Iterator& begin
			, Iterator const& end
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		static detail::http_headers_grammar_t<Iterator, ContextT> grammar_;
		grammar_.map_ = &map;
		grammar_.ctx_ = &ctx;

		return qi::parse(begin, end, grammar_, ctx);
	}

	template<class StringT, class ContextT>
	bool map_http_headers(
			  StringT const& headers
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		typename StringT::iterator b = headers.begin();
		return map_http_headers(b, headers.end(), map, ctx);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// full request/response parsers

	template<class Iterator, class ContextT>
	bool map_http_request(
			  Iterator& begin
			, Iterator const& end
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		return map_http_request_line(begin, end, ctx)
			&& map_http_headers(begin, end, map, ctx)
			;
	}

	template<class StringT, class ContextT>
	bool map_http_request(
			  StringT const& str
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		typename StringT::iterator begin = str.begin();
		return map_http_request(begin, str.end(), map, ctx);
	}

	template<class Iterator, class ContextT>
	bool map_http_response(
			  Iterator& begin
			, Iterator const& end
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		return map_http_response_line(begin, end, ctx)
			&& map_http_headers(begin, end, map, ctx)
			;
	}

	template<class StringT, class ContextT>
	bool map_http_response(
			  StringT const& str
			, http_header_mapping_t<ContextT> const& map
			, ContextT& ctx
			)
	{
		typename StringT::iterator begin = str.begin();
		return map_http_response(begin, str.end(), map, ctx);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__HTTP_HEADERS_IMPL_HPP_

