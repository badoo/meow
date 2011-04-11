////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__MAP_HPP_
#define MEOW_JUDY__MAP_HPP_

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <boost/type_traits/is_integral.hpp>

#include <meow/judy/judy.hpp>
#include <meow/judy/index.hpp>

#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////
//
// map owns it's own objects and indexes them with a key
// table support coming later

// table is the index that owns it's elements

	template<
		  class K
		, class V
		, class ValuePtr = boost::static_move_ptr<V>
		>
	struct map_t : private boost::noncopyable
	{
		typedef map_t 		self_t;
		typedef K 			key_t;
		typedef V 			value_t;
		typedef ValuePtr 	value_ptr;

	public:

		value_t* get(key_t const& k) const
		{
			value_t **v = i_.get(k);
			return v ? *v : NULL;
		}

		value_t** get_ptr(key_t const& k) const
		{
			return i_.get(k);
		}

		value_t** get_or_create_ptr(key_t const& k)
		{
			return i_.get_or_create(k);
		}

	public:

		// @desc: creates element with ownership
		// returns:
		//  NULL - no memory
		//  pointer to created or found object otherwise
		//   you can check was_inserted out param to find out if that was inserted

		static value_t* standard_ctor() { return new value_t; }

		value_t* get_or_create(key_t const& k, bool *was_inserted = NULL)
		{
			return get_or_create(k, &self_t::standard_ctor, was_inserted);
		}

		template<class CtorF>
		value_t* get_or_create(key_t const& k, CtorF const& ctor_fn, bool *was_inserted = NULL)
		{
			value_t **pp = i_.get_or_create(k);

			if (NULL == pp)
				return NULL;

			// nothrow from here

			value_t *& p = *pp;
			if (NULL == p) // new
			{
				if (NULL != was_inserted)
					*was_inserted = true;

				p = ctor_fn();
			}

			return p;
		}

		// @desc: inserts element gaining ownership
		//  returns: pair: <pointer to value_t inserted, value override happened>

		typedef std::pair<value_t*, bool> insert_result_t;

		insert_result_t insert(key_t const& k, value_ptr v)
		{
			value_t **pp = i_.get_or_create(k);
			
			if (NULL == pp)
				throw std::bad_alloc();

			// nothrow from here

			value_t *& p = *pp;
			bool const is_override = (NULL != p);

			if (is_override)
				delete_wrapper(p);

			p = v.release();

			return insert_result_t(p, is_override);
		}

		bool del(key_t const& k)
		{
			return i_.del(k);
		}

		void clear()
		{
			i_.for_each(&self_t::delete_wrapper_kv);
			i_.clear();
		}

	public:

		template<class Function>
		void for_each(Function const& function)
		{
			i_.for_each(function);
		}

	public:

		~map_t()
		{
			clear();
		}

	private:

		static void delete_wrapper(value_t *v)
		{
			value_ptr del_wrap(v);
		}

		static void delete_wrapper_kv(key_t const&, value_t *v)
		{
			delete_wrapper(v);
		}

	private:
		typedef index_t<key_t, value_t*> judy_index_t;
		judy_index_t i_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__MAP_HPP_

