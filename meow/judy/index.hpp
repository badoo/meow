////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2008 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__INDEX_HPP_
#define MEOW_JUDY__INDEX_HPP_

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <meow/utility/nested_type_checker.hpp>

#include "judy.hpp"
#include "judy_select.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class K, class V>
	struct index_t : private boost::noncopyable
	{
		typedef K key_t;
		typedef V value_t;

		BOOST_STATIC_ASSERT(sizeof(value_t) <= sizeof(word_t));

		typedef judy::judy_select<key_t> 			select_t;
		typedef typename select_t::operations_t 	j_ops;
		typedef typename select_t::handle_t 		handle_t;

	public:

		static value_t* error_value_p() { return reinterpret_cast<value_t*>(judy::j_error); }

		friend judy_t      & get_handle(index_t      & i) { return get_handle(i.j_); }
		friend judy_t const& get_handle(index_t const& i) { return get_handle(i.j_); }

	public:

		// @descr: finds the value and returns a pointer to it
		// @return:
		//  found: a pointer to the stored value
		//  not found: NULL
		value_t* get(key_t const& k) const
		{
			void *r = j_ops::get(j_, (typename j_ops::key_t)(k));

			BOOST_ASSERT((judy::j_error_p != r) && "get can't produce a malloc failure");
			return static_cast<value_t*>(r);
		}

		// @descr: gets the value, or creates a placeholder for where to put the value
		// @return:
		//  okay: pointer to the value that's been created or found
		//  		value is initialized to 0 if in was inserted
		//  		not modified otherwise
		//  fail: no memory -> NULL
		value_t* get_or_create(key_t const& k)
		{
			void *r = j_ops::get_or_create(j_, (typename j_ops::key_t)(k));

			BOOST_ASSERT((NULL != r) && "NULL return must not happen when inserting new value to index");
			return (judy::j_error_p != r) ? static_cast<value_t*>(r) : NULL;
		}

		// @descr: deletes a value with a given k_
		// @return:
		//  true: k_ has been found and deleted
		//  false: k_ not found
		bool del(key_t const& k)
		{
			return (1 == j_ops::del(j_, (typename j_ops::key_t)(k)));
		}

		// @descr: clears index, freing used memory
		void clear()
		{
			j_ops::free_array(j_);
		}

		MEOW_DEFINE_NESTED_TYPE_CHECKER(check_iteration, iteration);

		class iterator_t : public boost::iterator_facade<
							 						  iterator_t
													, value_t
													, boost::bidirectional_traversal_tag
													>
		{
			typedef iterator_t self_t;

			handle_t& j_;

			// word_t, and not key_t, since there is no automatic conversion for pointers
			//  word_t can convert to int in return but
			//  int* can't convert to word_t* in calls to j_ops::iteration:: functions
			word_t    k_;

			// the value pointer, cached from inside judy leaf, to not have to look up
			//  on dereference
			void*     v_;

		public:

			iterator_t(handle_t& j)
				: j_(j)
				, k_(0)
				, v_(NULL)
			{
				BOOST_STATIC_ASSERT(check_iteration<j_ops>::value);
			}

			key_t    key() const { return k_; }
			value_t& value() const { return *(value_t*)v_; }

			self_t& set_first()
			{
				k_ = 0;
				v_ = (value_t*)j_ops::iteration::first(j_, &k_);
				return *this;
			}

			self_t& set_last()
			{
				k_ = -1;
				v_ = (value_t*)j_ops::iteration::last(j_, &k_);
				return *this;
			}

		private:

			friend class boost::iterator_core_access;

			void increment()
			{
				v_ = (value_t*)j_ops::iteration::next(j_, &k_);
			}

			void decrement()
			{
				v_ = (value_t*)j_ops::iteration::prev(j_, &k_);
			}

			value_t& dereference()
			{
				return value();
			}

			template<class J>
			bool equal(J const& other) const
			{
				return v_ == other.v_;
			}
		};
		typedef iterator_t iterator;

		iterator begin() { return iterator(j_).set_first(); }
		iterator end() { return iterator(j_); }

	private:
		handle_t j_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class K, class V>
	V* index_get(index_t<K, V> const& idx, K const& k) { return idx.get(k); }

	template<class K, class V>
	V* index_get_or_create(index_t<K, V>& idx, K const& k) { return idx.get_or_create(k); }

	template<class K, class V>
	bool index_del(index_t<K, V>& idx, K const& k) { return idx.del(k); }

	template<class K, class V>
	void index_clear(index_t<K, V>& idx) { idx.clear(); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__INDEX_HPP_

