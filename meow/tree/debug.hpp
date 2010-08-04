////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__DEBUG_HPP_
#define MEOW_TREE__DEBUG_HPP_

#include "tree.hpp"

#include <meow/format/format.hpp>
#include <meow/format/metafunctions.hpp>
#include <meow/format/inserter/pointer.hpp>

#include <meow/gcc/demangle.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class S>
	inline S& debug_dump_impl(S& sink, node_t *node, size_t const level = 0)
	{
		using meow::format::fmt;

		switch (node->type())
		{
			case node_type::directory:
			{
				typedef directory_t::child_t 		child_t;
				typedef directory_t::child_range_t 	range_t;

				directory_t *dir = as_directory(node);
				range_t const child_r = dir->get_children();
				for (range_t::const_iterator i = child_r.begin(); i != child_r.end(); ++i)
				{
					child_t const& child = *i;

					if (level)
						for (size_t i = 0; i < level; ++i)
							fmt(sink, " ");

					node_t *child_node = child.ptr;

					switch (child_node->type())
					{
						case node_type::directory:
							fmt(sink, "{0}: {{ '{1}', {2} }\n",
									  enum_as_string(child_node->type())
									, child.name, child_node
									);

							debug_dump_impl(sink, child_node, level + 2);
						break;

						case node_type::file:
						{
							file_t *file_node = as_file(child_node);

							fmt(sink, "{0}: {{ '{1}', {2} }, type: {3}\n",
									  enum_as_string(child_node->type())
									, child.name, child_node
									, meow::gcc_demangle_name_tmp(file_node->type_info().name())
									);
						}
						break;
					}
				}
			}
			break;

			case node_type::file:
				if (level)
					for (size_t i = 0; i < level; ++i)
						fmt(sink, " ");

				file_t *f = as_file(node);
				fmt(sink, "file: {0}, type: {1}\n", node, meow::gcc_demangle_name_tmp(f->type_info().name()));
				break;
		}

		return sink;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class S>
	inline S& debug_dump(S& sink, node_t *node, size_t level = 1)
	{
		using meow::format::fmt;

		if (NULL != node)
		{
			fmt(sink, "----- tree dump from {0} -----\n", node);
			detail::debug_dump_impl(sink, node, level);
			fmt(sink, "----------------------------------------\n");
		}
		else
		{
			fmt(sink, "----- tree dump from NULL -----\n");
		}

		return sink;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__DEBUG_HPP_

