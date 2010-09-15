////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__PTR_CHAIN_HPP_
#define MEOW__PTR_CHAIN_HPP_

#include <boost/noncopyable.hpp>
#include <boost/intrusive/slist.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace ptr_chain_detail_
	{
		namespace bi = boost::intrusive;

		typedef bi::slist_base_hook<> hook_t;

		template<class T>
		struct ptr_chain_list_gen
		{
			typedef bi::slist<
						  T
						, bi::base_hook<hook_t>
						, bi::constant_time_size<true>
						, bi::cache_last<true>
						>
						type;
		};

	} // namespace ptr_chain_detail_

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T, class PtrT = boost::static_move_ptr<T> >
	struct ptr_chain_t : private boost::noncopyable
	{
		typedef ptr_chain_t 	self_t;
		typedef T 				value_t;
		typedef PtrT 			value_move_ptr;

	private:

		struct item_t : public ptr_chain_detail_::hook_t
		{
			value_move_ptr value;

			item_t(value_move_ptr& v)
				: value(move(v))
			{
			}
		};
		typedef typename ptr_chain_detail_::ptr_chain_list_gen<item_t>::type list_t;
		typedef boost::static_move_ptr<item_t> item_move_ptr;

	public:

		~ptr_chain_t()
		{
			this->clear();
		}

	public:

		typedef typename list_t::iterator 			iterator;
		typedef typename list_t::const_iterator 	const_iterator;

		iterator       begin()       { return l_.begin(); }
		const_iterator begin() const { return l_.begin(); }

		iterator       end()       { return l_.end(); }
		const_iterator end() const { return l_.end(); }

	public:
	
		bool empty() const { return l_.empty(); }
		size_t size() const { return l_.size(); }

		value_t* front() const
		{
			BOOST_ASSERT(!this->empty());
			return get_pointer(l_.front().value);
		}

		value_move_ptr grab_front()
		{
			BOOST_ASSERT(!this->empty());

			value_move_ptr result = move(l_.front().value);
			this->pop_front();
			return move(result);
		}

		void pop_front()
		{
			BOOST_ASSERT(!this->empty());
			item_move_ptr item_delete_wrapper(&l_.front());
			l_.pop_front();
		}

		void push_back(value_move_ptr v)
		{
			item_move_ptr item(new item_t(v));
			l_.push_back(*item);
			item.release();
		}

		void splice_after(const_iterator i, self_t& other)
		{
			l_.splice_after(i, other.l_);
		}

		void clear()
		{
			while (!this->empty())
				this->pop_front();
		}

	private:
		list_t l_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__PTR_CHAIN_HPP_

