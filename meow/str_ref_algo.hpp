////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2005 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_STR_REF_ALGO_HPP_
#define MEOW_STR_REF_ALGO_HPP_

#include <cstring> 		// for std::memchr
#include <algorithm> 	// for std::find

#include <boost/assert.hpp>
#include <boost/next_prior.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_cv.hpp>

#include "str_ref.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
	namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

		template<class Iterator, template<class> class Comp>
		struct cmp_to_any_t
		{
			typedef typename boost::iterator_value<Iterator>::type value_type;

			cmp_to_any_t(Iterator begin, Iterator end) : b_(begin), e_(end) {}
			bool operator()(value_type const& val) const
			{
				Comp<Iterator> comp;
				return comp(e_, std::find(b_, e_, val));
			}

		private:
			Iterator b_, e_;
		};

	#define DEFINE_COMPARATOR_MAKER(fun_name, comp_name)						\
		template<class Iterator>												\
		cmp_to_any_t<Iterator, comp_name> fun_name(Iterator b, Iterator e) {	\
			return cmp_to_any_t<Iterator, comp_name>(b, e);						\
		}																		\
	// ENDMACRO: DEFINE_COMPARATOR_MAKER

	DEFINE_COMPARATOR_MAKER(equal_to_any, std::not_equal_to)
	DEFINE_COMPARATOR_MAKER(not_equal_to_any, std::equal_to)

	#undef DEFINE_COMPARATOR_MAKER

////////////////////////////////////////////////////////////////////////////////////////////////
	} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline str_ref trim_left(str_ref in, str_ref const spaces = ref_lit(" \r\n\t"))
	{
		str_ref::iterator it = std::find_if(
				  in.begin(), in.end()
				, detail::not_equal_to_any(spaces.begin(), spaces.end())
			);
		return str_ref(it, in.end());
	}

	inline str_ref trim_right(str_ref in, str_ref const spaces = ref_lit(" \r\n\t"))
	{
		str_ref::reverse_iterator it = std::find_if(
				  in.rbegin(), in.rend()
				, detail::not_equal_to_any(spaces.begin(), spaces.end())
			);
		return str_ref(in.begin(), it.base());
	}

	// compares str_refs
	//  compare length is a min length of lhs and rhs
	//  returns if the prefixes match
	inline bool prefix_compare(str_ref lhs, str_ref rhs)
	{
		size_t const cmp_len = std::min(lhs.size(), rhs.size());

		str_ref const l = str_ref(lhs.begin(), cmp_len);
		str_ref const r = str_ref(rhs.begin(), cmp_len);
		return (l == r);
	}

	// compares str_refs at end
	//  compare length is a min length of lhs and rhs
	//  returns if the postfixes match
	inline bool postfix_compare(str_ref lhs, str_ref rhs)
	{
		size_t const cmp_len = std::min(lhs.size(), rhs.size());

		str_ref const l = str_ref(lhs.end() - cmp_len, lhs.end());
		str_ref const r = str_ref(rhs.end() - cmp_len, rhs.end());
		return (l == r);
	}

	// returns a sub 'string'
	//  starting at offset begin, ending at offset end
	//  precond: begin <= end, end <= str.size()
	//  postcond: the returned value refers to memory inside the original str_ref
	template<class CharT>
	inline string_ref<CharT> sub_str_ref(
								  string_ref<CharT> str
								, typename string_ref<CharT>::size_type begin
								, typename string_ref<CharT>::size_type end
								)
	{
		BOOST_ASSERT(begin <= end);
		BOOST_ASSERT(end <= str.size());
		return string_ref<CharT>(str.begin() + begin, str.begin() + end);
	}

	// returns path basename, doesn't handle trailing slashes
	inline str_ref basename(str_ref path, str_ref const slashes = ref_lit("/\\"))
	{
		str_ref::reverse_iterator it = std::find_if(
				  path.rbegin(), path.rend()
				, detail::equal_to_any(slashes.begin(), slashes.end())
			);
		return str_ref(it.base(), path.end());
	}

	// returns path dirname, doesn't handle trailing slashes
	inline str_ref dirname(str_ref path, str_ref const slashes = ref_lit("/\\"))
	{
		return str_ref(path.begin(), boost::prior(basename(path, slashes).begin()));
	}

	// returns basename with specific number of levels
	inline str_ref basename_ex(str_ref path, size_t nlevels, str_ref const slashes = ref_lit("/\\"))
	{
		str_ref tmp(path.end(), path.end());
		while (nlevels--)
		{
			tmp = dirname(str_ref(path.begin(), tmp.end()), slashes);
		}
		return str_ref(boost::next(tmp.end()), path.end());
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	inline string_ref<CharT> strstr_ex(string_ref<CharT> haystack, str_ref needle)
	{
		BOOST_STATIC_ASSERT((boost::is_same<typename boost::remove_cv<CharT>::type, char>::value));

		typedef string_ref<CharT> str_t;
		typedef typename str_t::iterator iterator_t;

		if (haystack.size() < needle.size())
			return str_t();

		if (0 == needle.size())
			return haystack;

		if (1 == needle.size())
		{
			iterator_t const p = (iterator_t)std::memchr(haystack.data(), needle[0], haystack.size());
			return (p) ? str_t(p, haystack.end()) : str_t();
		}

		iterator_t i = haystack.begin(), i_end = haystack.end();
		while (i < i_end)
		{
			i = (iterator_t)std::memchr(i, needle[0], haystack.end() - i);

			if (!i)
				return str_t();

			if ((haystack.end() - i) < std::ptrdiff_t(needle.size()))
				return str_t();

			if (0 == std::memcmp(i + 1, needle.data() + 1, needle.size() - 1))
				return str_t(i, haystack.end());

			++i;
		}

		return str_t();
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_STR_REF_ALGO_HPP_

