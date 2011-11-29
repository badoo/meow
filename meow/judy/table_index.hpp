////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__TABLE_INDEX_HPP_
#define MEOW_JUDY__TABLE_INDEX_HPP_

#include <cstdlib> 	// for malloc
#include <cstring> 	// for memcpy

#include <memory> 	// for std::auto_ptr
#include <vector> 	// for std::vector

#include <boost/type_traits/alignment_of.hpp>
#include <boost/range/iterator_range.hpp>

#include "judy.hpp"
#include "index.hpp"

#include <meow/utility/static_math.hpp>
#include <meow/str_ref.hpp>

/*
it's essentialy a smoothly-growing hash table(JudyL) with chains for conflict resolution

requirements:
- you will be storing pointers to your real values in the container
- your Value type, must have alignment of at least 2 bytes, and 4 is better
  (as we're using lower bits of the Judy values as type discrimitators in nodes)
- you can NOT store NULL in the container, doing this may result un undefined-behaviour
  (as we're storing pointers, we must dereference them to compare with passed keys for retrieval)

the main advantages are:
- much much smaller memory footprint than JudySL/JudyHS for most cases 
    (size difference directly proportional to the key length)
    due to not storing the key twice

- uses JudyL for the 'table', so you can approximate real memory usage easier if you got a good hash function

built as a combination of JudyL and complex leaves that morph into
 - direct pointer (most of them will be like this if you hash function is good)
 - compressed array pointer (size of the array is stored in lower bits of the real JudyL value)
 - vector pointer (full fledged C++ vector, for large nodes, improve your hash function if you got these)
 */

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class HashF>
	struct index_hash_base_t : private HashF
	{
		template<class K>
		size_t fn_hash(K const& k) const { return (*this)(k); }
	};

	template<class EqualF>
	struct index_equal_base_t : private EqualF
	{
		template<class K, class V>
		bool fn_equal(K const& k, V const& v) const { return (*this)(k, v); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class T
		, class BitmaskT
		, BitmaskT type_data_bitmask
		>
	struct mask_pointer_t
	{
		typedef mask_pointer_t 	self_t;
		typedef BitmaskT 		bitmask_t;

		// the one and only data we're holding, must be sizeof(void*)
		//  need this to be a union to do bit operations and have a pointer at the same time
		//  not daring to use bitfields because of possible endianess issues
		union {
			T 			*ptr;
			uintptr_t 	pointer_data_;
		};

		static void assert_self_size()
		{
			BOOST_STATIC_ASSERT(sizeof(T*) == sizeof(self_t));
		}
		
		static void assert_bitmasked_bits_are_zero(void *p)
		{
			BOOST_ASSERT(0 == ((uintptr_t)p & type_data_bitmask));
		}

	public:

		bitmask_t type() const { return bitmask_t(pointer_data_ & type_data_bitmask); }

		bitmask_t set_type(bitmask_t t)
		{
			pointer_data_ &= ~type_data_bitmask;
			pointer_data_ |= (t & type_data_bitmask);
			return t;
		}

	public:

		mask_pointer_t() : ptr(0) { assert_self_size(); }
		mask_pointer_t(T *p) : ptr(p) { assert_bitmasked_bits_are_zero(p); }

		self_t& operator=(self_t const& other)
		{
			ptr = other.ptr;
			return *this;
		}

		template<class U>
		self_t& operator=(U *p)
		{
			assert_bitmasked_bits_are_zero(p);
			ptr = (T*)p;
			return *this;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ValueT 					// value
		, class EqualF 					// test equality of key K and Value V
		, size_t alignment_sz_arg = 0 	// see the description in table_index_t
	>
	struct index_leaf_t : private index_equal_base_t<EqualF>
	{
		typedef index_leaf_t  self_t;
		typedef ValueT        value_t;
		typedef size_t        type_data_t;

	public:

		template<class T, size_t sz>
		struct alignment_helper
		{
			static size_t const alignment_sz = sz;
		};

		template<class T>
		struct alignment_helper<T, 0>
		{
			static size_t const alignment_sz = boost::alignment_of<T>::value;
		};

		// number of bits we can use for own purposes in user-exposed pointers
		static size_t const alignment_sz = alignment_helper<value_t, alignment_sz_arg>::alignment_sz;
		static size_t const alignment_bits = meow::static_log2<alignment_sz, meow::lower_bound_tag>::value;

		// TODO: implement the case for 1 too, this will mean
		//  that we have no array nodes, only vector and direct ones
		BOOST_STATIC_ASSERT(alignment_bits > 1);

		// special tpe information we'll be holding in lowest bits of user-sipplied pointers
		static type_data_t const type_data_bitmask = (size_t(1) << alignment_bits) - 1;
		static type_data_t const type_direct_ptr = 0; // don't change this, ever
		static type_data_t const type_vector_ptr = 1;
		static type_data_t const type_data_max = type_data_bitmask;

		typedef mask_pointer_t<value_t, type_data_t, type_data_bitmask> rich_pointer_t;

	private:
		rich_pointer_t ptr_;

		#define this_inner_ptr_ ptr_.ptr

		template<class T>
		static T* as_typed_ptr(rich_pointer_t const& p)
		{
			return reinterpret_cast<T*>(p.pointer_data_ & ~type_data_bitmask);
		}

	public:

		index_leaf_t() : ptr_(NULL) { BOOST_ASSERT(sizeof(self_t) == sizeof(ptr_)); }
		~index_leaf_t() { this->clear(); }

	public: // simple getters

		void* raw_ptr() const { return this_inner_ptr_; }

		template<class T>
		T* typed_raw_ptr() const { return reinterpret_cast<T*>(this_inner_ptr_); }

		type_data_t type() const { return ptr_.type(); }

		bool empty() const { return NULL == raw_ptr(); }

	private: // direct ptr ops

		value_t*  as_direct_ptr() const { return (value_t*)(this_inner_ptr_); }
		value_t** as_direct_ptr_ptr() const { return (value_t**)(&this_inner_ptr_); }

		void set_empty() { this_inner_ptr_ = NULL; }

		void assign_direct_ptr(value_t *v)
		{
			ptr_ = v;
			ptr_.set_type(self_t::type_direct_ptr);
		}

	private: // vector pointer ops

		typedef std::vector<value_t*> value_vector_t;

		value_vector_t* as_vector_ptr() const { return as_typed_ptr<value_vector_t>(ptr_); }

		static value_vector_t* vector_create_from_ptr(value_t *value)
		{
			std::auto_ptr<value_vector_t> v(new value_vector_t);
			v->reserve(2);
			v->push_back(value);
			v->push_back((value_t*)NULL);
			return v.release();
		}

		static value_vector_t* vector_create_from_array(value_t ** const ptr, size_t sz)
		{
			std::auto_ptr<value_vector_t> v(new value_vector_t);
			v->reserve(sz + 1);

			for (size_t i = 0; i < sz; ++i)
				v->push_back(ptr[i]);

			v->push_back((value_t*)NULL);

			return v.release();
		}

		void vector_destroy(value_vector_t *v)
		{
			delete v;
		}

	private: // compressed array ops

		struct array_data_t
		{
			value_t 	**ptr;
			size_t 		size;
		};

		array_data_t array_from_node() const
		{
			array_data_t const a = { as_typed_ptr<value_t*>(ptr_), ptr_.type() };
			return a;
		}

		static value_t** array_last_element_ptr(array_data_t const& arr)
		{
			BOOST_ASSERT(arr.size > 0);
			return &arr.ptr[arr.size - 1];
		}

		static array_data_t array_create_with_size(size_t sz)
		{
			array_data_t a;
			a.size = sz;
			a.ptr = (value_t**)std::malloc(sizeof(value_t*) * a.size);
			return a;
		}

		static array_data_t array_realloc_to_size(value_t **ptr, size_t sz)
		{
			array_data_t a;
			a.size = sz;
			a.ptr = (value_t**)std::realloc(ptr, sizeof(value_t) * sz);
			return a;
		}

		static void array_destroy(value_t **ptr)
		{
			std::free(ptr);
		}

		static array_data_t array_create_from_direct_ptr(value_t *direct_p)
		{
			array_data_t a = array_create_with_size(2);

			a.ptr[0] = direct_p;
			a.ptr[1] = NULL;

			return a;
		}

		static array_data_t array_create_as_copy(value_t **p, size_t sz)
		{
			array_data_t a = array_create_with_size(sz);
			std::memcpy(a.ptr, p, sizeof(*a.ptr) * sz);
			a.size = sz;

			return a;
		}

		static array_data_t array_add_empty_value(array_data_t const& old_a)
		{
			array_data_t a = array_realloc_to_size(old_a.ptr, old_a.size + 1);
			a.ptr[old_a.size] = NULL;
			return a;
		}

		static array_data_t array_drop_last_value(array_data_t const& old_a)
		{
			array_data_t a = array_realloc_to_size(old_a.ptr, old_a.size - 1);
			return a;
		}

	public: // node items as range

		typedef boost::iterator_range<value_t **> value_range_t;

		value_range_t decode_value_range() const
		{
			switch (ptr_.type())
			{
				case self_t::type_direct_ptr:
				{
					value_t **v = this->as_direct_ptr_ptr();
					return value_range_t(v, v + 1);
				}

				case self_t::type_vector_ptr:
				{
					value_vector_t *v = this->as_vector_ptr();
					return value_range_t(&*v->begin(), &*v->end());
				}

				default:
				{
					array_data_t const a = this->array_from_node();
					return value_range_t(a.ptr, a.ptr + a.size);
				}
			}
		}
	private: // node assign


		void assign_array_ptr(array_data_t const& arr)
		{
			BOOST_ASSERT(arr.size <= self_t::type_data_max);
			ptr_ = arr.ptr;
			ptr_.set_type(arr.size);
		}

		void assign_vector_ptr(value_vector_t *v)
		{
			ptr_ = (void*)v;
			ptr_.set_type(self_t::type_vector_ptr);
		}

	public:

		template<class K>
		value_t** get(K const& k) const
		{
			value_range_t const r = this->decode_value_range();

			for (typename value_range_t::const_iterator i = r.begin(), i_end = r.end()
				; i != i_end
				; ++i
				)
			{
				BOOST_ASSERT((NULL != *i) && "index_leaf_t value range element can't be NULL; forgot to initialize pointer you got from get_or_create() ?");
				if (self_t::fn_equal(k, **i))
					return i;
			}
			return NULL;
		}

		template<class K>
		value_t** get_or_create(K const& k)
		{
			if (this->empty())
				return this->as_direct_ptr_ptr();

			value_t **v = this->get(k);
			if (v)
				return v;

			switch (this->type())
			{
				case self_t::type_direct_ptr:
				{
					value_t* value = this->as_direct_ptr();

					if (type_data_max != type_vector_ptr) // check if we can expand to array node
					{
						array_data_t const new_a = array_create_from_direct_ptr(value);
						this->assign_array_ptr(new_a);
						return array_last_element_ptr(new_a);
					}
					else // otherwise expand to vector
					{
						value_vector_t *vec = vector_create_from_ptr(value);
						this->assign_vector_ptr(vec);
						return &vec->back();
					}
				}
				break;

				case self_t::type_vector_ptr:
				{
					value_vector_t *v = this->as_vector_ptr();
					v->push_back(NULL);
					return &v->back();
				}
				break;

				default: // compressed array
				{
					array_data_t const a = this->array_from_node();

					if (a.size < self_t::type_data_max) // got space to expand
					{
						array_data_t const new_a = array_add_empty_value(a);
						this->assign_array_ptr(new_a);
						return array_last_element_ptr(new_a);
					}
					else // otherwise expand to vector
					{
						value_vector_t *vec = vector_create_from_array(a.ptr, a.size);
						this->assign_vector_ptr(vec);
						return &vec->back();
					}
				}
				break;
			}

			BOOST_ASSERT(!"must not be reached");
		}

		template<class K>
		bool del(K const& k)
		{
			value_t **found_v = this->get(k);
			if (!found_v)
				return false;

			switch (this->type())
			{
				case self_t::type_direct_ptr:
					this->set_empty();
				break;

				case self_t::type_vector_ptr:
				{
					value_vector_t *v = this->as_vector_ptr();

					size_t const del_offset = found_v - &*v->begin();
					BOOST_ASSERT(del_offset < v->size());

					// shrink to direct ptr (only in the case of no arrays possible, 1 bit alignment)
					if (2 == v->size())
					{
						this->assign_direct_ptr((*v)[v->size() - del_offset - 1]);
						vector_destroy(v);
					}
					else
					{
						v->erase(v->begin() + del_offset);

						if (v->size() <= type_data_max)
						{
							array_data_t const new_a = array_create_as_copy(&*v->begin(), v->size());
							this->assign_array_ptr(new_a);
							vector_destroy(v);
						}
					}
				}
				break;

				default:
				{
					array_data_t a = this->array_from_node();

					size_t const del_offset = (found_v - a.ptr);
					BOOST_ASSERT(del_offset < a.size);

					// only 2 elements left, 1 is going to be deleted, so 1 left -> direct node
					if (2 == a.size)
					{
						this->assign_direct_ptr(a.ptr[a.size - del_offset - 1]);
						array_destroy(a.ptr);
					}
					else
					{
						// move the deleted value to the end
						using std::swap;
						swap(a.ptr[del_offset], a.ptr[a.size - 1]);

						// and just shrink array memory
						array_data_t const new_a = array_drop_last_value(a);
						this->assign_array_ptr(new_a);
					}
				}
				break;
			}

			return true;
		}

		void clear()
		{
			switch (this->type())
			{
				case self_t::type_direct_ptr:
				break;

				case self_t::type_vector_ptr:
					vector_destroy(as_vector_ptr());
				break;

				default:
					array_destroy(array_from_node().ptr);
				break;
			}

			this->set_empty();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ValueT 				// value
		, class HashF 				// hash(): key -> size_t
		, class EqualF 				// equal(): key, value -> bool
		, size_t alignment_sz = 0   // the alignment of the type we'll be referencing
									// you need it only when the target type is incomplete
									// and it's alignment can't be determined automaticaly
									// 0 - reserved value, to indicate default behaviour
		>
	struct table_index_t : private detail::index_hash_base_t<HashF>
	{
		typedef table_index_t  self_t;
		typedef ValueT         value_t;

		typedef detail::index_leaf_t<ValueT, EqualF, alignment_sz>  leaf_t;
		typedef judy::index_t<size_t, leaf_t>                       index_t;
		index_t j_;

	public:

		friend judy::judy_t get_handle(self_t &i)
		{
			return get_handle(i.j_);
		}

		friend judy::judy_t get_handle(self_t const &i)
		{
			return get_handle(i.j_);
		}

	private: // noncopyable

		table_index_t(table_index_t const&);
		table_index_t& operator=(table_index_t const&);

	public:

		table_index_t() {}
		~table_index_t() { this->clear(); }

	public:

		template<class K>
		value_t** get(K const& k) const
		{
			leaf_t *leaf_p = judy::index_get(j_, self_t::fn_hash(k));
			if (!leaf_p)
				return NULL;

			return leaf_p->get(k);
		}

		// complex insertion/retrieval function
		//
		//  finds or creates a new slot for key: k
		//  = returns a pointer the value stored in the slot
		//  + if new_v is not null, assigns new_v to the slot
		//  + is was_created is not null, sets it to true if the slot was created, false otherwise
		//
		//  WARNING: if the value has been created, you can't use any operations on the contained
		//            utill after you've initialized it with something non-NULL
		//            as the container needs the value to check key-value equality
		//
		template<class K>
		value_t** get_or_create(K const& k, value_t *new_v = NULL, bool *was_created = NULL)
		{
			leaf_t *leaf_p = judy::index_get_or_create(j_, self_t::fn_hash(k));
			if (!leaf_p)
				new (leaf_p) leaf_t;

			value_t **value = leaf_p->get_or_create(k);
			if (!value)
				return NULL;

			if (new_v)
				*value = new_v;

			if (was_created)
				*was_created = (NULL == *value);

			return value;
		}

		template<class K>
		bool del(K const& k)
		{
			size_t const k_hash = self_t::fn_hash(k);

			leaf_t *leaf_p = judy::index_get(j_, k_hash);
			if (!leaf_p) // not found
				return false;

			bool r = leaf_p->del(k);

			if (leaf_p->empty())
				judy::index_del(j_, k_hash);

			return r;
		}

		void clear()
		{
			size_t i = 0;
			judy::judy_t jj = get_handle(*this);

			for (leaf_t *l = (leaf_t*)JudyLFirst(jj, &i, PJE0)
				; l != NULL
				; l = (leaf_t*)JudyLNext(jj, &i, PJE0)
				)
			{
				l->clear();
			}

			judy::index_clear(j_);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class K, class V, class H, class E, size_t A>
	V** index_get(table_index_t<V, H, E, A> const& idx, K const& k) { return idx.get(k); }

	template<class K, class V, class H, class E, size_t A>
	V** index_get_or_create(table_index_t<V, H, E, A>& idx, K const& k) { return idx.get_or_create(k); }

	template<class K, class V, class H, class E, size_t A>
	bool index_del(table_index_t<V, H, E, A>& idx, K const& k) { return idx.del(k); }

	template<class V, class H, class E, size_t A>
	void index_clear(table_index_t<V, H, E, A>& idx) { return idx.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__TABLE_INDEX_HPP_

