////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2010+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__KV_MAPPING_HPP_
#define MEOW_MAPPING__KV_MAPPING_HPP_

#include <vector>
#include <utility> // for std::pair

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#ifdef MEOW_KV_MAPPING_ENABLE_JUDY_MAP
	#include <meow/judy/index.hpp>
#endif // MEOW_KV_MAPPING_ENABLE_JUDY_MAP

#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/qi/string/symbols.hpp>

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MEOW_KV_MAPPING_ENABLE_JUDY_MAP

	struct kv_mapping_map_judy_t
	{
		typedef str_ref key_t;

		typedef judy::index_t<key_t, void*> index_t;
		index_t j_;

		void map_add(key_t const& k, void *h)
		{
			void **hptr = j_.get_or_create(k);
			if (!hptr)
				throw std::bad_alloc();

			*hptr = h;
		}

		void* map_find(key_t const& k) const
		{
			void **hptr = j_.get(k);
			return (NULL != hptr) ? *hptr : NULL;
		}
	};

#endif // MEOW_KV_MAPPING_ENABLE_JUDY_MAP

	struct kv_mapping_map_spirit_parse_t
	{
		typedef str_ref key_t;

		struct field_names_parser_t 
			: public boost::spirit::qi::symbols<key_t::char_type, void*>
		{
		};
		field_names_parser_t field_parser_;

		void map_add(key_t const& k, void *h)
		{
			field_parser_.add(k.str(), h);
		}

		void* map_find(key_t const& k) const
		{
			str_ref::iterator begin = k.begin();
			void *found_handler = NULL;
			bool success = boost::spirit::qi::parse(begin, k.end(), field_parser_, found_handler);
			return success ? found_handler : NULL;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ContextT
		, class MapT = kv_mapping_map_spirit_parse_t
		>
	struct kv_mapping_t 
		: private boost::noncopyable
		, private MapT
	{
		typedef kv_mapping_t 	self_t;
		typedef self_t 			base_t; // for wrappers to use
		typedef boost::function<void(ContextT&, str_ref)> 			handler_t;
		typedef boost::function<void(ContextT&, str_ref, str_ref)> 	header_handler_t;

	private:
		std::vector<handler_t> 			handlers_;
		std::vector<header_handler_t> 	unknown_v_;
		std::vector<header_handler_t> 	any_v_;

	private:

		handler_t* insert_handler(handler_t const& handler)
		{
			handlers_.push_back(handler);
			return &handlers_.back();
		}

	public: // setters

		self_t& on_name(char const *name, handler_t const& h)
		{
			MapT::map_add(name, this->insert_handler(h));
			return *this;
		}

		self_t& on_any_name(header_handler_t const& h)
		{
			any_v_.push_back(h);
			return *this;
		}

		self_t& on_unknown(header_handler_t const& h)
		{
			unknown_v_.push_back(h);
			return *this;
		}

	public: // executor

		void call_handler(ContextT& ctx, std::pair<str_ref, str_ref> const& kv) const
		{
			this->call_handler(ctx, kv.first, kv.second);
		}

		void call_handler(ContextT& ctx, str_ref name, str_ref value) const
		{
			handler_t const *called_handler = (handler_t*)MapT::map_find(name);

			if (NULL != called_handler)
				return (*called_handler)(ctx, value);

			BOOST_FOREACH(header_handler_t const& h, unknown_v_)
				h(ctx, name, value);

			BOOST_FOREACH(header_handler_t const& h, any_v_)
				h(ctx, name, value);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct kv_mapping_executor_t
	{
		typedef void result_type;

		template<class MappingT, class ContextT, class ArgT>
		result_type operator()(MappingT const *m, ContextT *ctx, ArgT const& arg) const
		{
			m->call_handler(*ctx, arg);
		}

		template<class MappingT, class ContextT, class Arg1, class Arg2>
		result_type operator()(MappingT const *m, ContextT *ctx, Arg1 const& arg1, Arg2 const& arg2) const
		{
			m->call_handler(*ctx, arg1, arg2);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__KV_MAPPING_HPP_

