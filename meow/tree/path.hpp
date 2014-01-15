////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__PATH_HPP_
#define MEOW_TREE__PATH_HPP_

#include <cassert>
#include <vector>
#include <stdexcept> // std::logic_error

#include <boost/range/iterator_range.hpp>

#include <meow/smart_enum.hpp>
#include <meow/str_ref.hpp>
#include <meow/str_ref_algo.hpp>
#include <meow/tree/tree.hpp>
#include <meow/tree/tree_ops.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef std::vector<str_ref> path_parts_t;
	typedef boost::iterator_range<str_ref const*> path_parts_range_t;

	inline path_parts_t path_into_parts(str_ref const& path)
	{
		return split_ex(path, "/");
	}

	inline path_parts_range_t path_parts_as_range(path_parts_t const& parts, ssize_t off_b = 0, ssize_t off_e = 0)
	{
		return { parts.data() + off_b, parts.data() + parts.size() + off_e };
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_SMART_ENUM_STRUCT(path_node_type,
								((self,    "self"))
								((parent,  "parent"))
								((regular, "regular"))
								);

	inline path_node_type_t path_get_node_type(str_ref const& part)
	{
		assert(!part.empty());

		char const *p = part.begin();

		switch (*p++)
		{
			case '/':
				assert(!"slash can't be here!");
				break;

			case '.': // possible self

				if (part.end() == p)
					return path_node_type::self;

				switch (*p++)
				{
					case '.': // parent ?
						if (part.end() == p)
							return path_node_type::parent;
						else
							return path_node_type::regular; // random filename with 2 dots in front
						break;

					default:
						return path_node_type::regular; // random filename with 1 dot in front
				}

				break;

			default: // normal file
				return path_node_type::regular;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool path_is_absolute(str_ref const& path)
	{
		assert(!path.empty());
		return ('/' == path[0]);
	}

	inline path_parts_t path_normalize(path_parts_t const& parts)
	{
		path_parts_t r;
		r.reserve(parts.size());

		for(str_ref const& s : parts)
		{
			path_node_type_t const ntype = path_get_node_type(s);

			switch (ntype)
			{
				case path_node_type::self:
					break;
				case path_node_type::parent:
					if (!r.empty())
						r.pop_back();
					break;
				case path_node_type::regular:
					r.push_back(s);
					break;
			}
		}

		return r;
	}

	template<class StringT>
	inline std::string path_normalize(StringT const& path)
	{
		path_parts_t const parts = path_into_parts(path);
		path_parts_t const new_parts = path_normalize(parts);

		std::string r;
		r.reserve(path.size());

		if (path_is_absolute(path))
			r.append("/");

		for(str_ref const& s : new_parts)
		{
			bool const is_first = (&s == &new_parts[0]);
			if (!is_first)
				r.append("/");

			r.append(s.data(), s.size());
		}

		return r;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Function>
	inline void path_walk(node_t *root, str_ref const& path, Function const& function)
	{
		path_parts_range_t const parts_r = path_into_parts(path);

		for(str_ref const& name : parts_r)
		{
			if (!function(root, parts_r, name))
				break;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool do_path_get(tree::node_t *& root, path_parts_range_t const& parts_r, str_ref const& name)
	{
		path_node_type_t const ntype = tree::path_get_node_type(name);
		switch (ntype)
		{
			case path_node_type::self:
				break;

			case path_node_type::parent:
				if (root->has_parent())
					root = as_directory(root->parent());
				break;

			case path_node_type::regular:
			{
				if (tree::node_type::directory != root->type())
					throw std::logic_error("bad tree node type, can't search for path inside the file");

				root = as_directory(root)->get_child_value(name);
			}
			break;

			default:
				assert(!"can't be reached");
		}

		return (NULL != root);
	}

	inline node_t* get_path(node_t *root, path_parts_range_t const& parts_r)
	{
		for(str_ref const& name : parts_r)
		{
			if (!do_path_get(root, parts_r, name))
				break;
		}

		return root;
	}

	template<class StringT>
	inline tree::node_t* get_path(tree::node_t *root, StringT const& path)
	{
		return get_path(root, path_parts_as_range(path_into_parts(path)));
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

	inline void do_create_directories(directory_t*& root, str_ref const& name)
	{
		path_node_type_t const ntype = path_get_node_type(name);
		switch (ntype)
		{
			case path_node_type::self:
				break;

			case path_node_type::parent:
				if (root->has_parent())
					root = as_directory(root->parent());
				break;

			case path_node_type::regular:
			{
				node_t *node = root->get_child_value(name);
				if (NULL == node)
				{
					node = create_directory_p();
					root->add_child(name, node);
				}
				else
				{
					if (node_type::directory != node->type())
						throw std::logic_error("bad tree node type, can't create dirs inside the file");
				}

				root = as_directory(node);
			}
			break;

			default:
				assert(!"can't be reached");
		}
	}

	inline directory_t* tree_create_dir(directory_t *root, path_parts_range_t const& parts_r)
	{
		for(str_ref const& name : parts_r)
		{
			do_create_directories(root, name);
		}

		return root;
	}

	template<class StringT>
	inline directory_t* tree_create_dir(directory_t *root, StringT const& path)
	{
		path_parts_t const parts = path_into_parts(path);
		return tree_create_dir(root, path_parts_as_range(parts));
	}

	template<class StringT>
	inline file_t* tree_create_file(directory_t *root, StringT const& path, file_ptr file)
	{
		path_parts_t const parts = path_into_parts(path);

		if (parts.size() > 1)
			root = tree_create_dir(root, path_parts_as_range(parts, 0, -1));

		directory_t::child_t *child = root->get_child(parts.back());
		if (NULL == child)
		{
			root->add_child(parts.back(), file.get());
			return file.release();
		}
		else
		{
			if (node_type::file != child->ptr->type())
				throw std::logic_error("tree_create_file(): child found, but is not a file");

			return as_file(child->ptr);
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__PATH_HPP_

