////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2010+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__KV_MAPPING_HPP_
#define MEOW_MAPPING__KV_MAPPING_HPP_

#include <list>
#include <utility> // for std::pair

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/qi/string/symbols.hpp>

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace qi = boost::spirit::qi;

	template<class ContextT>
	struct kv_mapping_t : private boost::noncopyable
	{
		typedef kv_mapping_t self_t;
		typedef boost::function<void(ContextT&, str_ref)> handler_t;

		struct field_names_parser_t : public qi::symbols<char, handler_t*> {};
		field_names_parser_t field_parser_;

		std::list<handler_t> handlers_; // just to hold them, no searches are done
		handler_t unknown_handler_;

	public: // setters

		self_t& on_name(char const *name, handler_t const& h)
		{
			field_parser_.add(name, this->insert_handler(h));
			return *this;
		}

		self_t& on_unknown(handler_t const& h)
		{
			unknown_handler_ = h;
			return *this;
		}

	public: // executor

		void call_handler(ContextT& ctx, std::pair<str_ref, str_ref> const& kv) const
		{
			this->call_handler_impl(ctx, kv.first, kv.second);
		}

		void call_handler_impl(ContextT& ctx, str_ref name, str_ref value) const
		{
//			fprintf(stderr, "%s; ctx: %p, %.*s = '%.*s'\n", __func__, &ctx, name.c_length(), name.data(), value.c_length(), value.data());

			handler_t *called_handler = this->parse_tag(name);

			if (NULL != called_handler)
				return (*called_handler)(ctx, value);

			if (unknown_handler_)
				return unknown_handler_(ctx, name);
		}

	private:

		handler_t* insert_handler(handler_t const& handler)
		{
			handlers_.push_back(handler);
			return &handlers_.back();
		}

		handler_t* parse_tag(str_ref name) const
		{
			str_ref::iterator begin = name.begin();
			handler_t *found_handler = NULL;
			bool success = qi::parse(begin, name.end(), field_parser_, found_handler);
			return success ? found_handler : NULL;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct mapping_executor_t
	{
		typedef void result_type;

		template<class MappingT, class ContextT, class ArgT>
		result_type operator()(MappingT const *m, ContextT& ctx, ArgT const& arg) const
		{
			m->call_handler(ctx, arg);
		}

		template<class MappingT, class ContextT, class Arg1, class Arg2>
		result_type operator()(MappingT const *m, ContextT& ctx, Arg1 const& arg1, Arg2 const& arg2) const
		{
			m->call_handler(ctx, arg1, arg2);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__KV_MAPPING_HPP_

