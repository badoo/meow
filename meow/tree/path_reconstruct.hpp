////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__PATH_RECONSTRUCT_HPP_
#define MEOW_TREE__PATH_RECONSTRUCT_HPP_

#include <climits> // hopefuly PATH_MAX is there

#include <meow/tmp_buffer.hpp>
#include <meow/format/format.hpp>
#include <meow/format/sink/char_buffer.hpp>

#include "tree.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	// reconstruct path to this node from the root of the tree
	// this is a pretty time consuming operation actually if the tree is big enough
	//  so use with care and cache results if possible
	template<class S>
	inline S& reconstruct_path(S& sink, node_t *node)
	{
		using meow::format::fmt;

		// recursion terminator, tree root
		if (!node->parent())
			return sink;

		// parent is a directory, always
		directory_t *parent = as_directory(node->parent());

		reconstruct_path(sink, parent);

		directory_t::child_t *parent_to_node = parent->get_child(node);
		BOOST_ASSERT((NULL != parent_to_node) && "if we have a parent, then parent must be able to find us");
		fmt(sink, "/{0}", parent_to_node->name);

		return sink;
	}

	// convenience function, when you can just copy a sink (think sink == std::string)
	template<class S>
	S reconstruct_path(node_t *node)
	{
		S sink;
		return reconstruct_path(sink, node);
	}

	inline str_ref reconstruct_path_tmp(node_t *node, tmp_buffer<PATH_MAX> const& buf = tmp_buffer<PATH_MAX>())
	{
		meow::format::sink::char_buffer_sink_t sink(buf.get(), buf.size());
		reconstruct_path(sink, node);
		return str_ref(buf.get(), sink.size());
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__PATH_RECONSTRUCT_HPP_

