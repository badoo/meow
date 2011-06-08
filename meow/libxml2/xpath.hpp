////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBXML2__XPATH_HPP_
#define MEOW_LIBXML2__XPATH_HPP_

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <boost/range/iterator_range.hpp>

#include "core.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBXML2_OBJECT(XPath, Context,	xpath_ctx_ptr_t);
	MEOW_DEFINE_LIBXML2_OBJECT(XPath, Object,	xpath_object_ptr_t);

////////////////////////////////////////////////////////////////////////////////////////////////
	
	inline xpath_object_ptr_t xpath_eval_expression(char const *expr, xmlXPathContext& ctx) {
		return libxml2::acquire_move_ptr(xmlXPathEvalExpression(BAD_CAST expr, &ctx));
	}

	typedef boost::iterator_range<xmlNodePtr*> xpath_node_range_t;

	inline xpath_node_range_t fetch_node_range(xmlXPathObject const& obj) {
		xmlNodeSetPtr nodes = obj.nodesetval;
		if (NULL == nodes || 0 == nodes->nodeNr)
			return xpath_node_range_t((xmlNode**)NULL, (xmlNode**)NULL);

		return xpath_node_range_t(nodes->nodeTab, nodes->nodeTab + nodes->nodeNr);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBXML2__XPATH_HPP_

