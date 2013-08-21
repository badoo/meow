////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__XML_TREE_HPP_
#define MEOW_MAPPING__XML_TREE_HPP_

#include <cassert>
#include <vector>
#include <functional> // std::function
#include <stdexcept>  // std::runtime_error
#include <type_traits>

#include <meow/str_ref.hpp>
#include <meow/std_bind.hpp>

#include <meow/libxml2/core.hpp>
#include <meow/libxml2/tree.hpp>
#include <meow/libxml2/xpath.hpp>

#include <meow/mapping/actor.hpp> // for meow::mapping::dont_cast()

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct tree_mapping_error : public std::runtime_error
	{
		tree_mapping_error(xmlNodePtr n, char const *msg) 
			: std::runtime_error(msg)
			, node_(n)
		{
			assert(NULL != node_);
		}

		xmlNodePtr xml_node() const throw() { return node_; }

	private:
		xmlNodePtr node_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT>
	class xml_node_t
	{
		typedef	xml_node_t                               self_t;
		typedef std::function<void(ContextT&, str_ref)>	 handler_func_t;

	public: // ctors

		xml_node_t() : handler_() {}

		template<class Function>
		explicit xml_node_t(Function const& handler) : handler_(handler) {}

	public: // structure

		template<class Function>
		self_t& node(char const* xpath_expr, Function const& function)
		{
			return this->node(xpath_expr, function, meow::mapping::dont_cast());
		}

		template<class Function, class Caster>
		self_t& node(char const* xpath_expr, Function const& function, Caster const& cast)
		{
			static_assert(std::is_convertible<Function, handler_func_t>::value, "need compatible function type");

			typedef typename std::remove_reference<typename Caster::template result<ContextT>::type>::type ctx_type;
			return this->node(xpath_expr, cast, xml_node_t<ctx_type>(function));
		}

		template<class Caster, class CtxT>
		self_t& node(char const* xpath_expr, Caster const& cast, xml_node_t<CtxT> const& sub_node)
		{
			assert(NULL != xpath_expr);

			typedef typename Caster::template result<ContextT>::type cast_result_type;
			sub_nodes_.push_back(node_info_t(xpath_expr, std::bind<void>(sub_node, std::bind<cast_result_type>(cast, _1), _2, _3)));
			return *this;
		}

		self_t const& end_node() const { return *this; } // the dummy terminator for easier reading

	public:

		void operator()(ContextT& ctx, xmlXPathContextPtr xpath_ctx) const
		{
			(*this)(ctx, xpath_ctx, xpath_ctx->node);
		}

		void operator()(
			  ContextT& ctx					// the context to be modified
			, xmlXPathContextPtr xpath_ctx	// xpath context to search for child expressions from
			, xmlNodePtr node				// node that we're processing right now
		) const
		{
			call_handler(handler_, ctx, node);

			typedef typename sub_nodelist_t::const_iterator sub_nodes_iterator;
			for (sub_nodes_iterator sub_i = sub_nodes_.begin(); sub_i != sub_nodes_.end(); ++sub_i)
			{
				xpath_ctx->node = node;
				xpath_object_ptr_t xpath_obj = xpath_eval_expression(sub_i->xpath_expr, *xpath_ctx);
				xpath_node_range_t r = fetch_node_range(*xpath_obj);

//				fprintf(stderr, "(%p), eval'd expression: %s, found %zu objects\n", this, sub_i->xpath_expr, r.size());

				for (xpath_node_range_t::const_iterator i = r.begin(); i != r.end(); ++i)
					sub_i->trampoline(ctx, xpath_ctx, *i);
			}
		}

	private:

		template<class Handler, class CtxT>
		static void call_handler(Handler const& handler, CtxT& ctx, xmlNodePtr node)
		{
			if (!handler) return;
			assert(NULL != node);

//			fprintf(stderr, "node = %p, node->name = %s, node->type = %d\n", node, node->name, node->type);

			switch (node->type) {
				case XML_TEXT_NODE:
				case XML_CDATA_SECTION_NODE:
					handler(ctx, str_ref((char const*)node->content));
					break;
				case XML_ATTRIBUTE_NODE:
					call_handler(handler, ctx, node->children);
					break;
				default:
					throw tree_mapping_error(node, "unknown node type");
					break;
			}
		}

	private:

		struct node_info_t
		{
			typedef std::function<void(ContextT&, xmlXPathContextPtr, xmlNodePtr)> trampoline_t;

			char const		*xpath_expr;
			trampoline_t	trampoline;

			node_info_t(char const *e, trampoline_t const& t) : xpath_expr(e), trampoline(t) {}
		};

		typedef std::vector<node_info_t> sub_nodelist_t;

		handler_func_t const handler_;
		sub_nodelist_t sub_nodes_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// this is the main entry point to mapping xml tree to some program-defined objects

	template<class ContextT>
	void document_tree_map_ex(xmlDoc& doc, char const *xpath_expr, xml_node_t<ContextT> const& root_node, ContextT& ctx)
	{
		libxml2::xpath_ctx_ptr_t xpath_ctx = libxml2::acquire_move_ptr(xmlXPathNewContext(&doc));

		xml_node_t<ContextT> real_root;
		real_root.node(xpath_expr, meow::mapping::dont_cast(), root_node);
		real_root(ctx, xpath_ctx.get());
	}

	template<class ContextT>
	void document_tree_map(xmlDoc& doc, xml_node_t<ContextT> const& root_node, ContextT& ctx)
	{
		document_tree_map_ex(doc, "/", root_node, ctx);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__XML_TREE_HPP_

