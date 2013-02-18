////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__TREE_HPP_
#define MEOW_TREE__TREE_HPP_

#include <vector> 	// directory children holder
#include <typeinfo> // required for std::type_info

#include <boost/assert.hpp>
#include <boost/range/iterator_range.hpp> // dir children range

#include <meow/smart_enum.hpp>
#include <meow/std_unique_ptr.hpp>
#include <meow/str_ref.hpp>
#include <meow/str_copy.hpp>
#include <meow/utility/offsetof.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct convert_proxy_t
	{
		void *v;
		convert_proxy_t(void *vv) : v(vv) {}
		template<class T> operator T() const { return (T)v; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct node_t;
	typedef std::unique_ptr<node_t> node_ptr;

	struct directory_t;
	typedef std::unique_ptr<directory_t> directory_ptr;

	struct file_t;
	typedef std::unique_ptr<file_t> file_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_SMART_ENUM(node_type,	((file,       "file"))
										((directory,  "directory"))
										);

////////////////////////////////////////////////////////////////////////////////////////////////

	struct node_name_and_ptr_t
	{
		str_copy 	name;
		node_t 		*ptr;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct node_t
	{
		node_type_t 	type() const { return type_; }
		node_t* 		parent() const { return parent_; }
		bool 			has_parent() const { return (NULL != this->parent()); }

	public:

		void set_parent(node_t *new_p)
		{
			BOOST_ASSERT(!this->has_parent());
			parent_ = new_p;
		}

		void reset_parent()
		{
			this->set_parent(NULL);
		}

	public:

		node_t(node_type_t t)
			: type_(t)
			, parent_(NULL)
		{
//			printf("%s(%p, %s)\n", __PRETTY_FUNCTION__, this, enum_as_string(t));
		}

		virtual ~node_t()
		{
//			printf("%s(%p, %s)\n", __PRETTY_FUNCTION__, this, enum_as_string(type()));
		}

	private:
		node_type_t const 	type_;
		node_t 				*parent_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<node_type_t n_type>
	struct node_impl_t : public node_t
	{
		node_impl_t()
			: node_t(n_type)
		{
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// a node that can contain children (other directories and/or files)

	struct directory_t 
		: public node_impl_t<node_type::directory>
	{
		typedef directory_t 				self_t;
		typedef node_name_and_ptr_t 		child_t;

		typedef boost::iterator_range<child_t*> child_range_nc_t;
		typedef boost::iterator_range<child_t const*> child_range_t;

		virtual ~directory_t()
		{
			this->clear();
		}

	public:

		// does NOT check for duplicate names
		self_t* add_child(str_ref node_name, node_ptr node)
		{
			return this->add_child(node_name, node.release());
		}

		self_t* add_child(str_ref node_name, node_t *node)
		{
			node->set_parent(this); // potentialy overwriting parent, but it's okay
			this->impl_insert_child(node_name, node);

			return this;
		}

		void clear()
		{
			for (child_holder_t::iterator i = children_.begin(); i != children_.end(); ++i)
			{
				node_ptr dtor_wrap((*i).ptr);
			}
			children_.clear();
		}

	public:

		child_range_nc_t get_children()
		{
			return boost::make_iterator_range(&*children_.begin(), &*children_.end());
		}

		child_range_t get_children() const
		{
			return boost::make_iterator_range(&*children_.begin(), &*children_.end());
		}

		child_t* get_child(str_ref name)
		{
			for (child_holder_t::iterator i = children_.begin(); i != children_.end(); ++i)
			{
				child_t *child = &*i;
				if (child->name == name)
					return child;
			}
			return NULL;
		}

		child_t* get_child(node_t *child_ptr)
		{
			for (child_holder_t::iterator i = children_.begin(); i != children_.end(); ++i)
			{
				child_t *child = &*i;
				if (child->ptr == child_ptr)
					return child;
			}
			return NULL;
		}

		node_t* get_child_value(str_ref name)
		{
			child_t *child = this->get_child(name);
			return (child)
				? child->ptr
				: NULL
				;
		}

	private:

		void impl_insert_child(str_ref name, node_t *node)
		{
			child_t const ch = { name: str_copy(name), ptr: node };
			children_.push_back(ch);
		}

	private:

		typedef std::vector<child_t> child_holder_t;
		child_holder_t children_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct file_t 
		: public node_impl_t<node_type::file>
	{
		virtual std::type_info const& type_info() const = 0;
		virtual convert_proxy_t value() = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct file_impl_t : public file_t
	{
		typedef file_impl_t 	self_t;
		typedef T 				value_t;

		file_impl_t() : value_() {}
		file_impl_t(value_t const& v) : value_(v) {}

		value_t* local_value() { return &value_; }

	public:

		static self_t* self_from_value(value_t *v)
		{
			return MEOW_SELF_FROM_MEMBER(self_t, value_, v);
		}

	public:

		virtual std::type_info const& type_info() const { return typeid(value_t); }
		virtual convert_proxy_t value() { return convert_proxy_t(this->local_value()); }

	private:
		T value_;
	};

	template<class T>
	inline file_impl_t<T>* file_from_value(T *value)
	{
		return file_impl_t<T>::self_from_value(value);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline directory_t* create_directory_p() { return new directory_t; }
	inline directory_ptr create_directory() { return directory_ptr(create_directory_p()); }

	template<class T>
	inline file_t* create_file_p() { return new file_impl_t<T>(); }

	template<class T>
	inline file_t* create_file_p(T const& v) { return new file_impl_t<T>(v); }

	template<class T>
	inline file_ptr create_file() { return file_ptr(create_file_p<T>()); }

	template<class T>
	inline file_ptr create_file(T const& v) { return file_ptr(create_file_p<T>(v)); }

////////////////////////////////////////////////////////////////////////////////////////////////

	inline directory_t* as_directory(node_t *node)
	{
		BOOST_ASSERT(NULL != node);
		BOOST_ASSERT(node_type::directory == node->type());
		return static_cast<directory_t*>(node);
	}

	inline directory_t const* as_directory(node_t const *node)
	{
		BOOST_ASSERT(NULL != node);
		BOOST_ASSERT(node_type::directory == node->type());
		return static_cast<directory_t const*>(node);
	}

	inline file_t* as_file(node_t *node)
	{
		BOOST_ASSERT(NULL != node);
		BOOST_ASSERT(node_type::file == node->type());
		return static_cast<file_t*>(node);
	}

	template<class T>
	inline file_impl_t<T>* as_file_impl(node_t *node)
	{
		BOOST_ASSERT(NULL != node);
		BOOST_ASSERT(node_type::file == node->type());
		return static_cast<file_impl_t<T>*>(node);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__TREE_HPP_

