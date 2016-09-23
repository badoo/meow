////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__CHUNK_HPP_
#define MEOW__CHUNK_HPP_

#include <cstdint>     // uint32_t

#include <stdexcept>   // std::range_error
#include <iterator>    // std::reverse_iterator
#include <type_traits> // std::aligned_storage

#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/utility/in_place_factory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE: can't be used with incomplete types
//
// a simple noncopyable fixed-size chunk class with
//    iteration
//  , appending
//  , in place construction
template<
	  class T                // type of objects to store
	, size_t N               // max number of objects
	, class SizeT = uint32_t // type used to track sizes and lengths
>
struct chunk : private boost::noncopyable
{
	static_assert(N > 0, "array size must be >= 0");
	enum {
		    object_size       = sizeof(T)
		  , object_align      = alignof(T)
		  , static_size       = N
		  , static_size_bytes = object_size * static_size
	};

	using aligned_storage_t =
		typename std::aligned_storage<static_size_bytes, object_align>::type;

	using value_type             = T;
	using size_type              = SizeT;
	using difference_type        = std::ptrdiff_t;
	using reference              = value_type&;
	using const_reference        = value_type const&;
	using pointer                = value_type*;
	using const_pointer          = value_type const*;

	using iterator               = T*;
	using const_iterator         = T const*;
	using reverse_iterator       = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using range                  = boost::iterator_range<iterator>;
	using const_range            = boost::iterator_range<const_iterator>;
	using reverse_range          = boost::iterator_range<reverse_iterator>;
	using const_reverse_range    = boost::iterator_range<const_reverse_iterator>;

public: // iterators

	iterator       begin()       { return data_; }
	const_iterator begin() const { return data_; }
	iterator       end()       { return data_ + length_; }
	const_iterator end() const { return data_ + length_; }

	reverse_iterator       rbegin()       { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	reverse_iterator       rend()       { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

	range       as_range()       { return { begin(), end() }; }
	const_range as_range() const { return { begin(), end() }; }

	reverse_range       as_rev_range()       { return { rbegin(), rend() }; }
	const_reverse_range as_rev_range() const { return { rbegin(), rend() }; }

	size_type iter_off(iterator it) { range_assert(it); return it - begin(); }
	size_type iter_off(const_iterator it) const { range_assert(it); return it - begin(); }

public:

	bool empty() const { return 0 == size(); }
	bool full() const { return capacity() == size(); }
	size_type size() const { return length_; }
	size_type capacity() const { return static_size; }
	size_type available() const { return capacity() - size(); }

public: // accessors

	reference operator[](size_type n) { range_assert(n); return data_[n]; }
	const_reference operator[](size_type n) const { range_assert(n); return data_[n]; }

	reference at(size_type n) { range_check(n); return data_[n]; }
	const_reference at(size_type n) const { range_check(n); return data_[n]; }

	reference front() { return data_[0]; }
	const_reference front() const { return data_[0]; }

	reference back() { return data_[length_ - 1]; }
	const_reference back() const { return data_[length_ - 1]; }

	const_pointer data() const { return data_; }
	pointer c_array() { return data_; }

public: // modifiers

	void clear()
	{
		for (iterator i = begin(), i_end = end(); i != i_end; ++i)
			do_destroy(i);

		length_ = 0;
	}

	template<class InPlaceFactory>
	void push_back(InPlaceFactory const& f)
	{
		assert(!full());
		f.template apply<T>(static_cast<void*>(data_ + length_));
		++length_;
	}

	void push_back() // push default constructed object into the chunk
	{
		assert(!full());
		new (static_cast<void*>(data_ + length_)) T();
		++length_;
	}

	void push_back(const_reference value)
	{
		return push_back(boost::in_place(value));
	}

	void pop_back()
	{
		assert(!empty());
		do_destroy(data_ + --length_);
	}

	chunk& assign(chunk const& other)
	{
		clear();
		for (auto const& o : other)
			push_back(o);
		return *this;
	}

public:
	chunk() : length_(0) {} // NOTE: don't fill storage with zeroes
	chunk(chunk const& other) : length_(0) { assign(other); }

	chunk& operator=(chunk const& other)
	{
		if (this != &other)
			assign(other);
		return *this;
	}

	~chunk() { clear(); }

protected:

	void do_destroy(T* p) { p->~T(); }

	void range_assert(size_type n) const {
		assert((n < length_) && "chunk<>: offset out of range");
	}

	void range_assert(iterator i) const {
		assert((i >= begin()) && (i < end()) && "chunk<>: offset out of range");
	}

	void range_check(size_type n) const {
		if (n >= length_)
			throw std::range_error("chunk<>: offset out of range");
	}

protected:
	size_type length_;
	union {
		aligned_storage_t padding_;
		T                 data_[static_size];
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__CHUNK_HPP_

