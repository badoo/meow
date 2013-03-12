////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2010+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__KV_MAPPING_HPP_
#define MEOW_MAPPING__KV_MAPPING_HPP_

#include <string>
#include <vector>
#include <utility>      // pair
#include <functional>   // function
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <meow/str_ref.hpp>
#include <meow/hash/hash.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT>
	struct kv_mapping_t : private boost::noncopyable
	{
		using self_t            = kv_mapping_t;
		using base_t            = self_t; // for wrappers to use
		using handler_t         = std::function<void(ContextT&, str_ref)>;
		using header_handler_t  = std::function<void(ContextT&, str_ref, str_ref)>;
		using handlers_map_t    = std::unordered_map<str_ref, handler_t, meow::hash<str_ref> >;

	private:

		handlers_map_t                  handlers_;
		std::vector<header_handler_t> 	unknown_v_;
		std::vector<header_handler_t> 	any_v_;

	public: // setters

		self_t& on_name(str_ref name, handler_t const& h)
		{
			handlers_[name] = h;
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
			for(header_handler_t const& h : any_v_)
				h(ctx, name, value);

			auto i = handlers_.find(name);
			if (handlers_.end() == i)
			{
				for (header_handler_t const& h : unknown_v_)
					h(ctx, name, value);
			}
			else
			{
				handler_t const& h = (*i).second;
				h(ctx, value);
			}
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

