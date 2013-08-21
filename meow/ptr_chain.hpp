////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__PTR_CHAIN_HPP_
#define MEOW__PTR_CHAIN_HPP_

#include <type_traits>

#include <boost/noncopyable.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <meow/std_unique_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct ptr_chain_traits_default
	{
		typedef T                        value_t;
		typedef std::unique_ptr<value_t> value_move_ptr;

		static bool const constant_time_size = true;
		static bool const linear = false;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef boost::intrusive::slist_base_hook<>	ptr_chain_hook_t;

	template<bool v>
	struct ptr_chain_intrusive
	{
		static bool const value = v;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Traits
		, class IsIntrusive = ptr_chain_intrusive<false>
		, class Hook = ptr_chain_hook_t
	>
	struct ptr_chain_t : private boost::noncopyable
	{
		typedef ptr_chain_t 				self_t;
		typedef Hook 						hook_t;
		typedef typename Traits::value_t 	value_t;

		typedef typename Traits::value_move_ptr value_move_ptr;

	private:

		template<class V, class Tag> struct self_traits_t;

		template<class V>
		struct self_traits_t<V, ptr_chain_intrusive<false> >
		{
			struct item_t : public hook_t
			{
				value_move_ptr value;

				item_t(value_move_ptr& v) : value(move(v)) {}
			};

			static value_t* value_pointer(item_t const& i)
			{
				return i.value.get();
			}

			static value_move_ptr value_move(item_t& i)
			{
				return move(i.value);
			}

			template<class L>
			static void list_pop_empty_front(L& l)
			{
				item_t *item = &l.front();
				l.pop_front();
				delete item;
			}

			static item_t* item_from_value(value_move_ptr& v)
			{
				return new item_t(v);
			}
		};

		template<class V>
		struct self_traits_t<V, ptr_chain_intrusive<true> >
		{
			static_assert(std::is_base_of<hook_t, value_t>::value, "intrusive ptr chain node must inherit from hook");
			typedef value_t item_t;

			static value_t* value_pointer(item_t const& i)
			{
				return const_cast<value_t*>(&i);
			}

			static value_move_ptr value_move(item_t& i)
			{
				return value_move_ptr(&i);
			}

			template<class L>
			static void list_pop_empty_front(L& l)
			{
				l.pop_front();
			}

			static item_t* item_from_value(value_move_ptr& v)
			{
				return v.release();
			}
		};

		typedef self_traits_t<value_t, IsIntrusive> self_traits;
		typedef typename self_traits::item_t item_t;

		typedef boost::intrusive::slist<
					  item_t
					, boost::intrusive::base_hook<hook_t>
					, boost::intrusive::cache_last<true>
					, boost::intrusive::constant_time_size<Traits::constant_time_size>
					, boost::intrusive::linear<Traits::linear>
					>
					list_t;
	private:
		list_t l_;

	public: // iterators

		template<class I>
		struct my_iterator : public boost::iterator_facade<
							 						  my_iterator<I>
													, value_t
													, typename boost::iterator_traversal<I>::type
													, value_t*
													>
		{
			typedef I list_iterator_t;
			list_iterator_t i_;

			my_iterator() : i_() {}
			my_iterator(list_iterator_t i) : i_(i) {}

		private:

			friend class boost::iterator_core_access;

			void increment() { ++i_; }

			value_t* dereference() const { return self_traits::value_pointer(*i_); }

			template<class J>
			bool equal(J const& other) const { return (i_ == other.i_); }
		};

		typedef my_iterator<typename list_t::iterator> 			iterator;
		typedef my_iterator<typename list_t::const_iterator> 	const_iterator;

		iterator       begin()       { return iterator(l_.begin()); }
		const_iterator begin() const { return const_iterator(l_.begin()); }

		iterator       end()       { return iterator(l_.end()); }
		const_iterator end() const { return const_iterator(l_.end()); }

	public:

		~ptr_chain_t()
		{
			this->clear();
		}

	public:
	
		bool empty() const { return l_.empty(); }
		size_t size() const { return l_.size(); }

		value_t* front() const
		{
			assert(!this->empty());
			return self_traits::value_pointer(l_.front());
		}

		value_t* back() const
		{
			assert(!this->empty());
			return self_traits::value_pointer(l_.back());
		}

		value_move_ptr grab_front()
		{
			assert(!this->empty());

			value_move_ptr result = self_traits::value_move(l_.front());
			self_traits::list_pop_empty_front(l_);
			return move(result);
		}

		void pop_front()
		{
			this->grab_front();
		}

		void push_back(value_move_ptr v)
		{
			item_t *item = self_traits::item_from_value(v);
			l_.push_back(*item);
		}

		void append_chain(self_t& other)
		{
			l_.splice_after(l_.previous(l_.end()), other.l_);
		}

		void clear()
		{
			while (!this->empty())
				this->pop_front();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__PTR_CHAIN_HPP_

