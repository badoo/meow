////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__PATH_HPP_
#define MEOW_TREE__PATH_HPP_

#include "tree.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace path_private {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct path_node_type {
		enum type { self, parent, regular };
	};
	typedef path_node_type::type path_node_type_t;

	path_node_type_t parse_path_node(char const *path)
	{
		switch (*path++)
		{
			case '/':
				BOOST_ASSERT(!"slash can't be here!");
				break;

			case '.': // possible self
				switch (*path++)
				{
					case 0:	case '/': 
						return path_node_type::self;

					case '.': // possible parent
						switch (*path++)
						{
							case 0: case '/': return path_node_type::parent;
							default: return path_node_type::regular; // random filename with 2 dots in front
						}
						break;

					default: // random filename with 1 dot in front
						return path_node_type::regular;
				}
				break;

			default: // normal file
				return path_node_type::regular;
		}
	}

	node_t* get_path_relative(node_t *node, char const *path)
	{
//		fprintf(stderr, "%s[0]; node: %p, path: '%s'\n", __func__, node, path);

		if (!path)
			return node;

		// eat all adjacent slashes at start
		while (*path && '/' == *path) path++;

//		fprintf(stderr, "%s[1]; node: %p, path: '%s'\n", __func__, node, path);

		// return self if path is empty
		if (!path || !*path)
			return node;

		// now onto the node type
		path_node_type_t node_type = parse_path_node(path);

		switch (node_type)
		{
			case path_node_type::self:
				return get_path_relative(node, path + 1);
			case path_node_type::parent:
				if (node->has_parent())
					node = node->parent();
				return get_path_relative(node, path + 2);
			case path_node_type::regular:
			{
				char const *end = path;
				while (*end && ('/' != *end)) end++; // find where the slash is
				str_ref node_name = str_ref(path, end);
//				printf("node_name: %.*s\n", (int)node_name.size(), node_name.data());

				if (node_type::directory == node->type())
				{
					directory_t *dir = static_cast<directory_t*>(node);

					node_t *child = dir->get_child_value(node_name);
					if (!child)
						return NULL;

					return get_path_relative(child, end);
				}

				return NULL;
			}
			break;
		}
	}

	node_t* get_path_absolute(node_t *node, char const *path)
	{
		BOOST_ASSERT('/' == *path);
		return get_path_relative(get_root(node), ++path);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace path_private {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline node_t* get_path(node_t *root, char const *path)
	{
		BOOST_ASSERT(NULL != path);
		BOOST_ASSERT(NULL != root);

		return ('/' == *path)
			? path_private::get_path_absolute(root, path)
			: path_private::get_path_relative(root, path)
			;
	}

	inline directory_t* get_path_as_dir(node_t *root, char const *path)
	{
		return as_directory(get_path(root, path));
	}

	inline file_t* get_path_as_file(node_t *root, char const *path)
	{
		return static_cast<file_t*>(get_path(root, path));
	}

	inline convert_proxy_t get_path_as_file_value(node_t *root, char const *path)
	{
		node_t *found_node = get_path(root, path);
		if (!found_node)
			return convert_proxy_t(NULL);

		return as_file(found_node)->value();
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__PATH_HPP_

