////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>, based on code by Maxim Egorushkin
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_STR_REF_HPP_
#define MEOW_STR_REF_HPP_

#include <limits> 		// for std::numeric_limits<>
#include <string>		// for std::string and std::char_traits and stuff
#include <type_traits>

#ifndef BOOST_ASSERT_HPP
#define BOOST_ASSERT_HPP // disable, BOOST_ASSERT_MSG
#endif
#include <boost/assert.hpp>

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

		typedef typename std::remove_const<char_type>::type char_type_nc;

		typedef std::char_traits<char_type_nc> traits_type;
		typedef std::basic_string<typename traits_type::char_type, traits_type> string_type;

	private:
		struct unspecified_bool_helper { int dummy; };
		typedef int unspecified_bool_helper::* unspecified_bool_type;

	public:
		string_ref() : p_(0), n_(0) {}
		string_ref(char_type* p) : p_(p), n_(p ? traits_type::length(p) : 0) {}
		string_ref(char_type* p, size_type n) : p_(p), n_(n) {}
		string_ref(char_type* begin, char_type* end) : p_(begin), n_(end - begin) {}
		string_ref(string_type const& str) : p_(str.data()), n_(str.size()) {} // assume string data is contiguous

		self& assign(char_type *p, size_type n)
		{
			p_ = p;
			n_ = n;
			return *this;
		}

		self& assign(char_type *b, char_type *e)
		{
			BOOST_ASSERT(b <= e);
			p_ = b;
			n_ = (e - b);
			return *this;
		}

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
			else if(int r = traits_type::compare(a.p_, b.p_, (a.n_ < b.n_) ? a.n_ : b.n_))
				return r < 0;
			else
				return a.n_ < b.n_;
		}

		char_type_nc operator[](size_t offset) const
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
//  not including the terminating zero

	template<class Char, size_t len>
	inline string_ref<Char const> ref_lit(Char const(&lit)[len])
	{
		return string_ref<Char const>(lit, len - 1);
	}

	template<class Char, size_t len>
	inline string_ref<Char> ref_lit(Char (&lit)[len])
	{
		return string_ref<Char>(lit, len - 1);
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

	typedef string_ref<char const> str_ref;
	typedef string_ref<wchar_t const> wstr_ref;

	typedef string_ref<char> buffer_ref;
	typedef string_ref<wchar_t> wbuffer_ref;

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_BUFFER_ALLOCA(sz) meow::buffer_ref((char*)alloca(sz), sz)
#define MEOW_BUFFER_ALLOCA_W(sz) meow::w_buffer_ref((wchar_t*)alloca(sz * sizeof(wchar_t)), sz)

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_STR_REF_HPP_

