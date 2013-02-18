////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__FIXL_STRING_HPP_
#define MEOW__FIXL_STRING_HPP_

#include <cstring> 					// for std::memcpy, std::strlen

#include <boost/assert.hpp> 		// for BOOST_ASSERT
#include <boost/integer.hpp> 		// for boost::uint_value_t<>

#include <meow/str_ref.hpp> 				// for meow::str_ref
#include <meow/utility/static_math.hpp> 	// for meow::min_covering_length

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////
// 
// this is a simple class holding a fixed-max-length string by value, with no memory allocation
// the special thing about it is: it holds a null-terminated string 
//  AND has constant-time complexity length() function
// this is achieved for storing length 'after' the actual string data in memory
// it saves us a bit of space as well as giving easy way to hand out c-style strings
//

	template<size_t MaxLength>
	struct fixl_string_t
	{
	public:

		// 2-step length size detection
		// 1. what length size we need to hold the data?
		// 2. extend the data size if we need to maintain proper alignment for the length
		// 3. calculate length size again, for new, extended data length
		typedef typename boost::uint_value_t<MaxLength>::fast step1_length_t;
		static size_t const step1_length = meow::min_covering_length<MaxLength, sizeof(step1_length_t)>::value;
		
		typedef typename boost::uint_value_t<step1_length>::fast length_t;
		static size_t const max_data_length = step1_length;

	private:

		// make sure it's a pod
		struct {
			char 		data_[max_data_length];
			length_t 	length_;
		};

		length_t read_length() const
		{
			return max_data_length - length_;
		}

		void store_length(length_t const length)
		{
			length_ = max_data_length - length;
		}

	public:

		fixl_string_t()
		{
			assign(NULL, 0);
		}

		explicit fixl_string_t(char const *str)
		{
			assign(str, std::strlen(str));
		}

		fixl_string_t(char const *str, size_t const length)
		{
			assign(str, length);
		}

		fixl_string_t(fixl_string_t const& other)
		{
			assign(other.c_str(), other.length());
		}

		fixl_string_t& operator=(fixl_string_t const& other)
		{
			this->assign(other.c_str(), other.sz_length());
			return *this;
		}

		fixl_string_t& operator=(str_ref const& str)
		{
			this->assign(str);
			return *this;
		}

	public:

		void assign(char const *str, size_t const length)
		{
			BOOST_ASSERT(length <= max_data_length);

			std::memcpy(data_, str, length);
			this->set_length(length);
		}

		void assign(str_ref const& str)
		{
			this->assign(str.data(), str.size());
		}

		void clear()
		{
			this->set_length(0);
		}

		char* 		c_str()       { return data_; }
		char const* c_str() const { return data_; }

		length_t 	length() const { return read_length(); }
		size_t 		max_length() const { return max_data_length; }

		int 		c_length() const { return length(); }
		size_t 		sz_length() const { return length(); }

        // a more low level function, use with care
        void        set_length(length_t const new_len)
		{
			data_[new_len] = 0x0; // null terminate
			store_length(new_len);
		}

    public:

        typedef char*           iterator;
        typedef char const*     const_iterator;

        iterator begin() { return data_; }
        iterator end() { return data_ + length(); }

        const_iterator begin() const { return data_; }
        const_iterator end() const { return data_ + length(); }

	public:

		typedef void(fixl_string_t::* bool_type)();

		operator bool_type() const
		{
			return (read_length() > 0)
				? &fixl_string_t::clear
				: NULL
				;
		}

	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<size_t len>
	bool operator==(fixl_string_t<len> const& l, fixl_string_t<len> const& r)
	{
		return (l.length() == r.length())
			? (0 == std::memcmp(l.c_str(), r.c_str(), r.sz_length()))
			: false
			;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<size_t l>
	fixl_string_t<l>& fixl_string_assign(fixl_string_t<l>& to, str_ref from)
	{
		to.assign(from.data(), from.size());
	}

	template<size_t l>
	buffer_ref as_buffer_ref(fixl_string_t<l>& s) { return buffer_ref(s.c_str(), s.length()); }

	template<size_t l>
	str_ref as_str_ref(fixl_string_t<l> const& s) { return str_ref(s.c_str(), s.length()); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__FIXL_STRING_HPP_

