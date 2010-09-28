////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__STATIC_NODE_HPP_
#define MEOW_TREE__STATIC_NODE_HPP_

#include <stdexcept> // for std::logic_error

#include <boost/assert.hpp>

// formating exception text
#include <meow/format/format.hpp>
#include <meow/format/format_tmp.hpp>
#include <meow/format/sink/std_string.hpp>

#include "tree.hpp"
#include "path.hpp"
#include "path_reconstruct.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace detail {

		template<class T>
		static void checked_init(T& value, directory_t*)
		{
			BOOST_ASSERT(!"static_node_t<>: trying to assign a directory to value");
		}

		static void checked_init(directory_t*& value, directory_t *dir)
		{
			value = as_directory(dir);
		}

		static void checked_init(directory_t*& value, file_t*)
		{
			BOOST_ASSERT(!"static_node_t<directory_t>: trying to assign a value to directory");
		}

		template<class T>
		static void checked_init(T*& value, file_t *file)
		{
			value = as_file_impl<T>(file)->local_value();
		}
	}

	template<class T>
	struct static_node_t
	{
		T      & operator*()       { return *value_; }
		T const& operator*() const { return *value_; }

		T      * operator->()       { return value_; }
		T const* operator->() const { return value_; }

		friend T* get_handle(static_node_t const& n) { return n.value_; }

	public:

		static_node_t(node_t *root, char const *path)
		{
			init(root, path);
		}

		static_node_t(node_t& root, char const *path)
		{
			init(&root, path);
		}

	private:

		void init_from_node(node_t *node)
		{
			switch (node->type())
			{
				case node_type::directory:
					detail::checked_init(value_, as_directory(node));
					break;

				case node_type::file:
					detail::checked_init(value_, as_file(node));
					break;
			}
		}

		void init(node_t *root, char const *path)
		{
			node_t *node = get_path(root, path);

			if (NULL == node)
			{
				throw std::logic_error(format::fmt_str(
							  "static_node_t<>::init(): root: \"{1}\", can't find path: \"{0}\""
							, reconstruct_path_tmp(root)
							, path
							));
			}

			this->init_from_node(node);
		}

	private:
		T *value_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__STATIC_NODE_HPP_

