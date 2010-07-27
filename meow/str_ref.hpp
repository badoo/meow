////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>, based on code by Maxim Egorushkin
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_STR_REF_HPP_
#define MEOW_STR_REF_HPP_

#include <iosfwd> 		// for std::ostream fwd
#include <algorithm>	// for std::min
#include <limits> 		// for std::numeric_limits<>
#include <string>		// for std::string and std::char_traits and stuff
#include <boost/assert.hpp>
#include <boost/type_traits/remove_const.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Char>
	class string_ref
	{
	public:
		typedef string_ref	self;
		typedef size_t		size_type;
		typedef Char		char_type;
		typedef char_type	value_type;

		typedef char_type*			iterator;
		typedef char_type const*	const_iterator;

		typedef std::reverse_iterator<iterator>			reverse_iterator;
		typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;

		typedef std::char_traits<typename boost::remove_const<char_type>::type> traits_type;
		typedef std::basic_string<typename traits_type::char_type, traits_type> string_type;

	private:
		struct unspecified_bool_helper { int dummy; };
		typedef int unspecified_bool_helper::* unspecified_bool_type;

	public:
		string_ref() : p_(0), n_(0) {}
		string_ref(char_type* p) : p_(p), n_(traits_type::length(p)) {}
		string_ref(char_type* p, size_type n) : p_(p), n_(n) {}
		string_ref(char_type* begin, char_type* end) : p_(begin), n_(end - begin) {}
		string_ref(string_type const& str) : p_(str.data()), n_(str.size()) {} // assume string data is contiguous

		iterator begin() const { return p_; }
		iterator end() const { return p_ + n_; }

		reverse_iterator rbegin() const { return reverse_iterator(end()); }
		reverse_iterator rend() const { return reverse_iterator(begin()); }

		size_type size() const { return n_; }
		size_type length() const { return n_; }
		int c_length() const { BOOST_ASSERT(n_ < size_type(std::numeric_limits<int>::max)); return n_; }

		bool empty() const { return !n_; }
		char_type* data() const { return p_; }
		string_type str() const { return string_type(p_, n_); }

		friend bool operator==(self const& a, self const& b)
		{
			if(a.n_ != b.n_)
				return false;
			else if(a.p_ == b.p_)
				return true;
			else
				return 0 == traits_type::compare(a.p_, b.p_, a.n_);
		}

		friend bool operator!=(self const& a, self const& b)
		{
			return !(a == b);
		}

		friend bool operator<(self const& a, self const& b)
		{
			if(!b.p_)
				return false;
			else if(!a.p_)
				return true;
			else if(int r = traits_type::compare(a.p_, b.p_, std::min(a.n_, b.n_)))
				return r < 0;
			else
				return a.n_ < b.n_;
		}

		char_type operator[](size_t offset) const
		{
			BOOST_ASSERT(offset < length());
			return p_[offset];
		}

		// NOTE: gives a warning when ch is already const-qualified
		//  	 might want to add enable_if<> here, but lazy
		operator string_ref<char_type const>() const
		{
			return string_ref<char_type const>(p_, n_);
		}

		operator unspecified_bool_type() const {
			return empty()
				? 0
				: &unspecified_bool_helper::dummy
				;
		}

	private:
		char_type* p_;
		size_type n_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// make str_ref from a string constant

	template<class Char, size_t len>
	inline string_ref<Char const> ref_lit(Char const(&lit)[len])
	{
		return string_ref<Char const>(lit, static_cast<size_t>(len - 1));
	}

	template<class Char, size_t len>
	inline string_ref<Char> ref_lit(Char (&lit)[len])
	{
		return string_ref<Char>(lit, static_cast<size_t>(len - 1));
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// make str_ref from a simple plain array

	template<class Char, size_t len>
	inline string_ref<Char const> ref_array(Char const(&arr)[len])
	{
		return string_ref<Char const>(arr, len);
	}

	template<class Char, size_t len>
	inline string_ref<Char> ref_array(Char (&arr)[len])
	{
		return string_ref<Char>(arr, len);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT, class Traits>
	inline
	std::basic_ostream<CharT, Traits>&
	operator<<(std::basic_ostream<CharT, Traits>& o, string_ref<CharT> const& s)
	{
		return o.write(s.data(), s.size());
	}

	template<class CharT, class Traits>
	inline
	std::basic_ostream<CharT, Traits>&
	operator<<(std::basic_ostream<CharT, Traits>& o, string_ref<CharT const> const& s)
	{
		return o.write(s.data(), s.size());
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef string_ref<char const> str_ref;
	typedef string_ref<wchar_t const> wstr_ref;

	typedef string_ref<char> buffer_ref;
	typedef string_ref<wchar_t> wbuffer_ref;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_STR_REF_HPP_

