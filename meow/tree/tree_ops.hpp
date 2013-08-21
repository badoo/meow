////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__TREE_OPS_HPP_
#define MEOW_TREE__TREE_OPS_HPP_

#include "tree.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	// walks up the tree till reaching the root
	//  and returns it
	inline directory_t* get_root(node_t *node)
	{
		assert(NULL != node);

		while (node->parent())
			node = node->parent();

		return as_directory(node);
	}

	inline directory_t* get_enclosing_dir(node_t *node)
	{
		assert(NULL != node);

		switch (node->type())
		{
			case node_type::directory:
				return as_directory(node);

			case node_type::file:
				return (node->parent())
					? as_directory(node->parent())
					: NULL
					;
		}

		assert(!"must not be reached");
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__TREE_OPS_HPP_

