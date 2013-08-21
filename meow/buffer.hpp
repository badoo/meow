////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__BUFFER_HPP_
#define MEOW__BUFFER_HPP_

#include <cstring> // memcpy
#include <cstdlib> // malloc

#include <exception>   // bad_alloc
#include <type_traits> // remove_const

#include <boost/noncopyable.hpp>

#include <meow/str_ref.hpp>
#include <meow/std_unique_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct buffer_impl_t : private boost::noncopyable
	{
		typedef buffer_impl_t                           self_t;
		typedef typename std::remove_const<CharT>::type char_t;

	private:
		// physical dimensions
		char_t * begin_;
		char_t * end_; // <-- past the end ptr

		static char_t* do_malloc(size_t const n_chars)
		{
			return (char_t*)malloc(n_chars * sizeof(char_t));
		}

		static char_t* do_realloc(char_t *p, size_t const n_chars)
		{
			return (char_t*)realloc(p, n_chars * sizeof(char_t));
		}

		static void do_free(char_t *p)
		{
			free(p);
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

		buffer_impl_t(char_t *b, size_t sz, size_t first_off = 0, size_t last_off = size_t(-1))
			: begin_(b)
			, end_(begin_ + sz)
			, first(begin_ + first_off)
			, last(begin_ + ((size_t(-1) == last_off) ? sz : last_off))
		{
			assert(NULL != begin_);
			invariant_check();
		}

		~buffer_impl_t()
		{
			self_t::do_free(begin_);
		}

	public:

		char_t* begin() const { return begin_; }
		char_t* end() const { return end_; }

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

	public:

		void resize_to(size_t new_sz)
		{
			char_t *new_begin = self_t::do_realloc(this->begin_, new_sz);
			assert(NULL != new_begin);

			this->first = new_begin + (this->first - this->begin_);
			this->last = new_begin + (this->last - this->begin_);
			this->begin_ = new_begin;
			this->end_ = new_begin + new_sz;

			this->invariant_check();
		}

	private:
		void invariant_check() { assert((begin_ <= first) && (first <= last) && (first <= end_)); }
	};

	using buffer_t        = buffer_impl_t<char>;
	using w_buffer_t      = buffer_impl_t<wchar_t>;

	using buffer_ptr      = std::unique_ptr<buffer_t>;
	using buffer_move_ptr = buffer_ptr;

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

	inline void append_to_buffer(buffer_t& buf, void const *data, size_t data_len)
	{
		if (buf.free_size() < data_len)
			buf.resize_to(buf.size() + data_len - buf.free_size());

		copy_to_buffer(buf, data, data_len);
	}

	inline buffer_t& buffer_move_used_part_to_front(buffer_t& buf)
	{
		if (buf.begin() != buf.first)
		{
			str_ref const remainder_s = buf.used_part();
			std::memmove(buf.begin(), remainder_s.begin(), remainder_s.size());
			buf.reset_first(buf.begin());
			buf.reset_last(buf.begin() + remainder_s.size());
		}

		return buf;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline buffer_move_ptr buffer_create_with_data(void const *data, size_t data_len)
	{
		assert(data_len > 0);

		buffer_move_ptr buf = create_buffer(data_len);
		copy_to_buffer(*buf, data, data_len);
		return move(buf);
	}

	inline buffer_move_ptr buffer_create_with_data(void const *data, void const *data_end)
	{
		assert(data < data_end);
		return buffer_create_with_data(data, size_t((char const*)data_end - (char const*)data));
	}

	inline buffer_move_ptr buffer_create_with_msg(str_ref const& msg)
	{
		return buffer_create_with_data(msg.data(), msg.size());
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__BUFFER_HPP_

