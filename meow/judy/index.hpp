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

#include <boost/bind.hpp>

#include <meow/movable_handle.hpp>
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

		// @descr: deletes a value with a given key
		// @return:
		//  true: key has been found and deleted
		//  false: key not found
		bool del(key_t const& k)
		{
			return (1 == j_ops::del(j_, (typename j_ops::key_t)(k)));
		}

		// @descr: clears index, freing used memory
		void clear()
		{
			j_ops::free_array(j_);
		}

		// @ descr: calls function(k, value*) for each item in the index
		struct call_proxy_t
		{
			typedef void result_type;

			template<class Function>
			result_type operator()(Function const& f, key_t const& k, void *v) const
			{
				return f(k, *static_cast<value_t*>(v));
			}
		};

		MEOW_DEFINE_NESTED_TYPE_CHECKER(check_iteration, iteration);

		template<class Function>
		void for_each(Function const& function)
		{
			BOOST_STATIC_ASSERT(check_iteration<j_ops>::value); // clang++ doesn't like: && (bool)"j_ops must have iteration enabled");
			j_ops::iteration::for_each(j_, boost::bind(call_proxy_t(), boost::cref(function), _1, _2));
		}

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

