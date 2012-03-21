////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__FOR_EACH_HPP_
#define MEOW_TREE__FOR_EACH_HPP_

#include <boost/foreach.hpp>

#include <meow/tree/tree.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Function>
	void for_each_file(node_t *root, Function const& function)
	{
		if (node_type::directory == root->type())
		{
			directory_t *dir = as_directory(root);

			typedef directory_t::child_range_t range_t;

			range_t const r = dir->get_children();
			for (range_t::const_iterator i = r.begin(); i != r.end(); ++i)
			{
				node_t *child = (*i).ptr;
				for_each_file(child, function);
			}
		}
		else
		{
			function(as_file(root));
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// node_ptr tree_map(tree_root, directory_transform_function, file_transform_function, int initial_level_value)
	//
	// similarly to map() operation for lists this function
	//  creates a new tree from the existing one
	//  by applying functions to directories and files the tree consists of
	//
	//  @returns: the new tree
	//
	//  directory function:
	//   directory_ptr func_for_directory(directory_t *existing_d, int level);
	//
	//   @returns: the new directory, can return empty pointer, in that case the whole subtree is not processed
	//
	//  file function:
	//   file_ptr func_for_file(file_t *existing_f, int level);
	//
	//   @returns: the new file, can return empty pointer, in that case it won't appear in result
	//


	template<class DirectoryF, class FileF>
	node_ptr tree_map(
			  node_t *root
			, DirectoryF const& directory_fn
			, FileF const& file_fn
			, int level = 0
			)
	{
		switch (root->type())
		{
			case node_type::directory:
			{
				directory_t *d = as_directory(root);
				directory_ptr new_d = directory_fn(d, level);

				if (new_d)
				{
					BOOST_FOREACH(directory_t::child_t& child, d->get_children())
					{
						node_ptr new_node = tree_map(child.ptr, directory_fn, file_fn, level + 1);
						new_d->add_child(child.name.ref(), move(new_node));
					}
				}

				return move(new_d);
			}

			case node_type::file:
				return file_fn(as_file(root), level);

			default:
				BOOST_ASSERT(!"can't be reached");
				break;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	//
	// tree_for_each(tree_root, directory_handler_fn, file_handler_fn, root_node_name, initial_level_value)
	//
	// iterate the tree depth-first and
	//  call directory_handler_fn for directories
	//  call file_handler_fn for files
	//
	//  directory function:
	//   void func_for_directory(directory_t *existing_d, str_ref node_name, int level);
	//
	//  file function:
	//   void func_for_file(file_t *existing_f, str_ref node_name, int level);
	//

	template<class DirectoryF, class FileF>
	void tree_for_each(
			  node_t *node
			, DirectoryF const& directory_fn
			, FileF const& file_fn
			, str_ref const& name = str_ref()
			, int level = 0
			)
	{
		switch (node->type())
		{
			case node_type::directory:
			{
				directory_t *dir = as_directory(node);

				directory_fn(dir, name, level);

				BOOST_FOREACH(directory_t::child_t& child_r, dir->get_children())
				{
					tree_for_each(child_r.ptr, directory_fn, file_fn, child_r.name.ref(), level + 1);
				}
			}
			break;

			case node_type::file:
				file_fn(as_file(node), name, level);
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__FOR_EACH_HPP_


