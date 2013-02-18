////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__CHUNK_HPP_
#define MEOW__CHUNK_HPP_

#include <cstdint>

#include <stdexcept> // std::range_error
#include <iterator>  // std::reverse_iterator

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/type_traits/alignment_of.hpp>
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
	  class T						// type of objects to store
	, std::size_t N					// max number of objects
	, class SizeT = std::size_t		// type used to track sizes and lengths
>
struct chunk : private boost::noncopyable
{
	BOOST_STATIC_ASSERT(N > 0);
	enum {
		  object_size = sizeof(T)
		, object_align = boost::alignment_of<T>::value
		, chunk_size = N
		, chunk_size_chars = object_size * chunk_size
	};

	enum { static_size = N };

	typedef typename boost::aligned_storage<
			  chunk_size_chars
			, object_align
		> aligned_storage_t;

	typedef T				value_type;
	typedef T*				pointer;
	typedef T const*		const_pointer;
	typedef T&				reference;
	typedef T const&		const_reference;
	typedef SizeT			size_type;
	typedef std::ptrdiff_t	difference_type;

	typedef T*			iterator;
	typedef T const*	const_iterator;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	typedef boost::iterator_range<iterator>			range_t;
	typedef boost::iterator_range<const_iterator>	const_range_t;

	typedef boost::iterator_range<reverse_iterator>			reverse_range_t;
	typedef boost::iterator_range<const_reverse_iterator>	const_reverse_range_t;

public: // iterators

	iterator begin() { return get_data(); }
	iterator end() { return get_data() + length_; }

	const_iterator begin() const { return get_data(); }
	const_iterator end() const { return get_data() + length_; }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

	range_t as_range() { return range_t(this->begin(), this->end()); }
	const_range_t as_range() const { return const_range_t(this->begin(), this->end()); }

	reverse_range_t as_rev_range() { return reverse_range_t(this->rbegin(), this->rend()); }
	const_reverse_range_t as_rev_range() const { return const_reverse_range_t(this->rbegin(), this->rend()); }

	size_type iter_off(iterator it) { range_assert(it); return it - begin(); }
	size_type iter_off(const_iterator it) const { range_assert(it); return it - begin(); }

public:

	bool empty() const { return 0 == size(); }
	bool full() const { return capacity() == size(); }
	size_type size() const { return length_; }
	size_type capacity() const { return chunk_size; }
	size_type available() const { return capacity() - size(); }
	
public: // accessors
	reference operator[](size_type n) { range_assert(n); return get_data()[n]; }
	const_reference operator[](size_type n) const { range_assert(n); return get_data()[n]; }

	reference at(size_type n) { range_check(n); return get_data()[n]; }
	const_reference at(size_type n) const { range_check(n); return get_data()[n]; }

	reference front() { get_data()[0]; }
	const_reference front() const { return get_data()[0]; }

	reference back() { return get_data()[length_ - 1]; }
	const_reference back() const { return get_data()[length_ - 1]; }

	const_pointer data() const { return get_data(); }
	pointer c_array() { return get_data(); }

public: // modifiers

	void clear() {
		for (iterator i = begin(), i_end = end(); i != i_end; ++i)
			do_destroy(i);

		length_ = 0;
	}

	template<class InPlaceFactory>
	void push_back(InPlaceFactory const& f) {
		BOOST_ASSERT(!this->full());
		f.template apply<T>(static_cast<void*>(get_data() + length_));
		++length_;
	}

	void push_back() { // push default constructed object into the chunk
		BOOST_ASSERT(!this->full());
		new (static_cast<void*>(get_data() + length_)) T();
		++length_;
	}

	void push_back(const_reference value) {
		return this->push_back(boost::in_place(value));
	}

	void pop_back() {
		BOOST_ASSERT(!this->empty());
		do_destroy(get_data() + --length_);
	}

	chunk& assign(chunk const& other)
	{
		this->clear();
		for (const_iterator i = other.begin(), e = other.end(); i != e; ++i)
			this->push_back(*i);
		return *this;
	}

public:
	chunk() : length_(0) {} // NOTE: don't fill storage with zeroes
	chunk(chunk const& other) : length_(0) { this->assign(other); }

	chunk& operator=(chunk const& other)
	{
		if (this != &other)
			this->assign(other);
		return *this;
	}

	~chunk() { this->clear(); }

protected:

	void do_destroy(T* p) { p->~T(); }

	pointer get_data() { return static_cast<pointer>(data_.address()); }
	const_pointer get_data() const { return static_cast<const_pointer>(data_.address()); }

	void range_assert(size_type n) const {
		BOOST_ASSERT((n < length_) && "chunk<>: offset out of range");
	}

	void range_assert(iterator i) const {
		BOOST_ASSERT((i >= begin()) && (i < end()) && "chunk<>: offset out of range");
	}

	void range_check(size_type n) const {
		if (n >= length_)
			throw std::range_error("chunk<>: offset out of range");
	}

protected:
	size_type 			length_;
	aligned_storage_t 	data_;
};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__CHUNK_HPP_

