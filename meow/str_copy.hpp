////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BMA_PROXY__STR_COPY_HPP_
#define BMA_PROXY__STR_COPY_HPP_

#include <memory> // std::uninitialized_copy
#include <limits> // std::numeric_limits<>
#include <string> // std::char_traits, std::string

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/utility/enable_if.hpp>

#include <meow/str_ref.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class CharT
		, class Traits = std::char_traits<typename boost::remove_const<CharT>::type>
		>
	struct string_copy
	{
		typedef string_copy self_t;
		typedef CharT 		char_type;
		typedef char_type	value_type;
		typedef size_t		size_type;
		typedef Traits		traits_type;
		typedef std::basic_string<typename traits_type::char_type, traits_type> string_type;

		typedef typename boost::remove_const<char_type>::type char_type_nc;

		typedef string_ref<char_type> string_ref_t;
		typedef boost::static_move_ptr<char_type[]> move_pointer_t;

	private:
		struct unspecified_bool_helper { int dummy; };
		typedef int unspecified_bool_helper::* unspecified_bool_type;

		// antoxa: char_type can't be const for the sake of my sanity
		BOOST_STATIC_ASSERT(!boost::is_const<char_type>::value);

		template<class U>
		struct can_copy_from
		{
			enum {
				  convert = boost::is_convertible<U, char_type>::value
				, same = boost::is_same<U, char_type>::value
				, value = convert || same
			};
		};

	public: // ctor/dtor/assign

		string_copy() : p_(), n_(0) {}
		string_copy(char_type const *p, size_type n) { this->init_copy(p, n); }
		string_copy(char_type const *b, char_type const *e) { BOOST_ASSERT(b <= e); this->init_copy(b, (e - b)); }
		string_copy(string_type const& s) { this->init_copy(s.data(), s.size()); }

		explicit string_copy(string_ref<char_type_nc> const& s) { this->init_copy((char_type*)s.data(), s.size()); }
		explicit string_copy(string_ref<char_type_nc const> const& s) { this->init_copy((char_type*)s.data(), s.size()); }

		string_copy(self_t const& other) { this->init_copy(other.data(), other.size()); }

		template<class U>
		string_copy(
				  string_copy<U> const& other
				, typename boost::enable_if_c<can_copy_from<U>::value, self_t>::type * = 0
				)
		{
			this->init_copy(other.data(), other.size());
		}

		template<class U>
		self_t& operator=(string_ref<U> const& other)
		{
			self_t tmp(other);
			p_.swap(tmp.p_);
			std::swap(n_, tmp.n_);
			return *this;
		}

		self_t& operator=(self_t const& other)
		{
			return ((*this) = other.ref());
		}

		template<class U>
		typename boost::enable_if_c<can_copy_from<U>::value, self_t>::type&
		operator=(string_copy<U> const& other)
		{
			return ((*this) = other.ref());
		}

	public: // iterators

		typedef char_type*			iterator;
		typedef char_type const*	const_iterator;

		typedef std::reverse_iterator<iterator>			reverse_iterator;
		typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;

		iterator begin() const { return data(); }
		iterator end() const { return begin() + n_; }

		reverse_iterator rbegin() const { return reverse_iterator(end()); }
		reverse_iterator rend() const { return reverse_iterator(begin()); }

	public: // simple getters

		char_type* data() const { return p_.get(); }
		string_ref_t ref() const { return string_ref_t(begin(), end()); }

		bool empty() const { return !n_; }
		size_type size() const { return n_; }
		size_type length() const { return n_; }
		int c_length() const { BOOST_ASSERT(n_ < size_type(std::numeric_limits<int>::max)); return n_; }

	public: // modifiers

		void reset() { p_.reset(); n_ = 0; }
		void clear() { reset(); }

	public: // operators

		operator unspecified_bool_type() const
		{
			return empty()
				? 0
				: &unspecified_bool_helper::dummy
				;
		}

		friend bool operator<(self_t const& l, self_t const& r)
		{
			if(!r.data())
				return false;
			else if(!l.data())
				return true;
			else if(int v = traits_type::compare(l.data(), r.data(), std::min(l.n_, r.n_)))
				return v < 0;
			else
				return l.n_ < r.n_;
		}

		char_type operator[](size_t offset) const
		{
			BOOST_ASSERT(offset < size());
			return data()[offset];
		}

	private:

		void init_copy(char_type_nc const *p, size_type n)
		{
			if (0 == n)
			{
				reset();
				return;
			}

			move_pointer_t pp(reinterpret_cast<char_type*>(new char[n * sizeof(char_type)]));
			std::uninitialized_copy(p, p + n, pp.get());
			p_.swap(pp);
			n_ = n;
		}

	private:
		move_pointer_t 	p_;
		size_type  		n_;
	};

	typedef string_copy<char> str_copy;

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class C>
	bool operator==(string_copy<C> const& l, string_copy<C> const& r)
	{
		if(l.size() != r.size())
			return false;
		else if(l.data() == r.data())
			return true;
		else
			return 0 == string_copy<C>::traits_type::compare(l.data(), r.data(), l.size());
	}

	template<class C1, class C2>
	bool operator==(string_copy<C1> const& l, string_ref<C2> const& r)
	{
		return l.ref() == r;
	}

	template<class C1, class C2>
	bool operator==(string_ref<C1> const& r, string_copy<C2> const& l)
	{
		return l.ref() == r;
	}

	template<class C1, class C2>
	bool operator!=(string_copy<C1> const& l, string_copy<C2> const& r)
	{
		return !(l == r);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT, class Traits>
	struct type_tunnel<string_copy<CharT, Traits> >
	{
		static string_ref<CharT> call(string_copy<CharT, Traits> const& s) { return s.ref(); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BMA_PROXY__STR_COPY_HPP_

