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

#include <meow/str_ref.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct buffer_t : private boost::noncopyable
	{
	private:
		// physical dimensions
		char * begin_;
		char * end_; // <-- past the end ptr

	public:

		// logical dimensions (filled with meaningful data)
		char *first;
		char *last; // <-- past last char ptr

	public:

		buffer_t(size_t sz)
			: begin_((char*)std::malloc(sz))
			, end_(begin_ + sz)
			, first(begin_)
			, last(first)
		{
			if (NULL == begin_)
				throw std::bad_alloc();
		}

		~buffer_t()
		{
			std::free(begin_);
		}
		
		char* begin() const { return begin_; }
		char* end() const { return end_; }

		void resize_to(size_t new_sz)
		{
			char *new_begin = (char*)std::realloc(begin_, new_sz);
			if (NULL == new_begin)
				throw std::bad_alloc();

			// fix pointers now, preserving the relative positions
			first = new_begin + (first - begin_);
			last = new_begin + (last - begin_);
			begin_ = new_begin;
			end_ = begin_ + new_sz;

			invariant_check();
		}

		util::buffer_ref ref() const { return util::buffer_ref(begin_, end_); }
		util::buffer_ref used_part() const { return util::buffer_ref(first, last); }
		util::buffer_ref free_part() const { return util::buffer_ref(last, end_); }

		size_t size() const { return size_t(end_ - begin_); }
		size_t used_size() const { return size_t(last - first); }
		size_t free_size() const { return size_t(end_ - last); }

		bool is_full() const { return (last == end_); }
		bool is_empty() const { return (first == last); }

		void reset_first(char *p) { first = p; invariant_check(); }
		void reset_last(char *p) { last = p; invariant_check(); }

		void advance_first(size_t offset) { first += offset; invariant_check(); }
		void advance_last(size_t offset) { last += offset; invariant_check(); }

	private:
		void invariant_check() { BOOST_ASSERT((begin_ <= first) && (first <= last) && (first <= end_)); }
	};
	typedef boost::static_move_ptr<buffer_t> buffer_move_ptr;

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

