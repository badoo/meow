////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__BUFFER_HPP_
#define MEOW__BUFFER_HPP_

#include <cstring> // memcpy
#include <cstdlib> // malloc

#include <exception>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <meow/str_ref.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct buffer_impl_t : private boost::noncopyable
	{
		typedef buffer_impl_t 								self_t;
		typedef typename boost::remove_const<CharT>::type 	char_t;

	private:
		// physical dimensions
		char_t * begin_;
		char_t * end_; // <-- past the end ptr

		static char_t* do_malloc(size_t const n_chars)
		{
			return (char_t*)std::malloc(n_chars * sizeof(char_t));
		}

		static char_t* do_realloc(char_t *p, size_t const n_chars)
		{
			return (char_t*)std::realloc(p, n_chars * sizeof(char_t));
		}

		static void do_free(char_t *p)
		{
			std::free(p);
		}

	public:

		// logical dimensions (filled with meaningful data)
		char_t *first;
		char_t *last; // <-- past last element ptr

	public:

		buffer_impl_t(size_t sz)
			: begin_(self_t::do_malloc(sz))
			, end_(begin_ + sz)
			, first(begin_)
			, last(first)
		{
			if (NULL == begin_)
				throw std::bad_alloc();
		}

		~buffer_impl_t()
		{
			self_t::do_free(begin_);
		}
		
		char_t* begin() const { return begin_; }
		char_t* end() const { return end_; }

		void resize_to(size_t new_sz)
		{
			char_t *new_begin = self_t::do_realloc(begin_, new_sz);
			if (NULL == new_begin)
				throw std::bad_alloc();

			// fix pointers now, preserving the relative positions
			first = new_begin + (first - begin_);
			last = new_begin + (last - begin_);
			begin_ = new_begin;
			end_ = begin_ + new_sz;

			invariant_check();
		}

		buffer_ref ref() const { return buffer_ref(begin_, end_); }
		buffer_ref used_part() const { return buffer_ref(first, last); }
		buffer_ref free_part() const { return buffer_ref(last, end_); }

		size_t size() const { return size_t(end_ - begin_); }
		size_t used_size() const { return size_t(last - first); }
		size_t free_size() const { return size_t(end_ - last); }

		bool full() const { return (last == end_); }
		bool empty() const { return (first == last); }

		void clear() { first = last = begin_; invariant_check(); }

		void reset_first(char_t *p) { first = p; invariant_check(); }
		void reset_last(char_t *p) { last = p; invariant_check(); }

		void advance_first(size_t offset) { first += offset; invariant_check(); }
		void advance_last(size_t offset) { last += offset; invariant_check(); }

	private:
		void invariant_check() { BOOST_ASSERT((begin_ <= first) && (first <= last) && (first <= end_)); }
	};

	typedef buffer_impl_t<char> 				buffer_t;
	typedef buffer_impl_t<wchar_t> 				w_buffer_t;
	typedef boost::static_move_ptr<buffer_t> 	buffer_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

	inline buffer_move_ptr create_buffer(size_t sz)
	{
		return buffer_move_ptr(new buffer_t(sz));
	}

	inline void copy_to_buffer(buffer_t& buf, void const *data, size_t data_len)
	{
		std::memcpy(buf.last, data, data_len);
		buf.advance_last(data_len); // will assert if we overflowed
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__BUFFER_HPP_

