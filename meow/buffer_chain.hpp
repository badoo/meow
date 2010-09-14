////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__BUFFER_CHAIN_HPP_
#define MEOW__BUFFER_CHAIN_HPP_

#include <boost/noncopyable.hpp>
#include <boost/intrusive/slist.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace buffer_detail_
	{
		namespace bi = boost::intrusive;

		typedef bi::slist_base_hook<> hook_t;

		template<class T>
		struct list_gen
		{
			typedef bi::slist<
						  T
						, bi::base_hook<hook_t>
						, bi::constant_time_size<true>
						, bi::cache_last<true>
						>
						type;
		};

	} // namespace buffer_detail_

////////////////////////////////////////////////////////////////////////////////////////////////

	struct buffer_chain_t : private boost::noncopyable
	{
		struct item_t : public buffer_detail_::hook_t
		{
			buffer_move_ptr buf;

			item_t(buffer_move_ptr& b)
				: buf(move(b))
			{
			}
		};
		typedef buffer_detail_::list_gen<item_t>::type list_t;
		typedef boost::static_move_ptr<item_t> item_move_ptr;

	public:

		~buffer_chain_t()
		{
			this->clear();
		}

	public:
	
		bool empty() const { return l_.empty(); }
		size_t size() const { return l_.size(); }

		buffer_t* front() const
		{
			BOOST_ASSERT(!this->empty());
			return get_pointer(l_.front().buf);
		}

		void pop_front()
		{
			BOOST_ASSERT(!this->empty());
			item_move_ptr item_delete_wrapper(&l_.front());
			l_.pop_front();
		}

		void push_back(buffer_move_ptr b)
		{
			item_move_ptr item(new item_t(b));
			l_.push_back(*item);
			item.release();
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

#endif // MEOW__BUFFER_CHAIN_HPP_

