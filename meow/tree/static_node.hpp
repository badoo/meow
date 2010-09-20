////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__STATIC_NODE_HPP_
#define MEOW_TREE__STATIC_NODE_HPP_

#include <boost/assert.hpp>

#include "tree.hpp"
#include "path.hpp"

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
			BOOST_ASSERT(NULL != node);

			this->init_from_node(node);
		}

	private:
		T *value_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__STATIC_NODE_HPP_
