////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_IMPL_HPP_
#define MEOW_HASH__HASH_IMPL_HPP_

#include <cstring> // std::strlen

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <meow/hash/hash_fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	enum { hash_def_initval = 0x9e3779b9 /* golden ratio */ };

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	// when you just have stuff in memory
	inline hash_result_t
	hash_blob(
			  void const *blob
			, size_t const size
			, hash_result_t const initval = detail::hash_def_initval
			)
	{
		return hash_impl<hash_default_tag>::hash_blob(blob, size, initval);
	}

	template<class T>
	inline hash_result_t hash_object(T const& obj);
	// THIS_FUNCTION_IS_NOT_FIT_FOR_GENERIC_OBJECTS_PLEASE_DEFINE_YOUR_OWN_THING

	inline hash_result_t hash_object(char const *s, hash_result_t const initval = detail::hash_def_initval)
	{
		return hash_blob(s, std::strlen(s), initval);
	}

	inline hash_result_t hash_object(signed char const *s, hash_result_t const initval = detail::hash_def_initval)
	{
		return hash_object((char const*)s, initval);
	}

	inline hash_result_t hash_object(unsigned char const *s, hash_result_t const initval = detail::hash_def_initval)
	{
		return hash_object((char const*)s, initval);
	}

	template<class CharT>
	inline hash_result_t
	hash_object(
			  string_ref<CharT const> const& s
			, hash_result_t const initval = detail::hash_def_initval
			)
	{
		return hash_blob(s.data(), s.size() * sizeof(CharT), initval);
	}

	template<class T, std::size_t N>
	inline hash_result_t
	hash_object(
			  T const (&data)[N]
			, hash_result_t const initval = detail::hash_def_initval
			)
	{
		return hash_object(ref_lit(data), initval);
	}

	template<class ArithmeticT>
	inline
	typename boost::enable_if_c<
		  boost::is_arithmetic<ArithmeticT>::value && (sizeof(ArithmeticT) >= sizeof(uint32_t))
		, hash_result_t
	>::type
	hash_object(ArithmeticT value, hash_result_t const initval = detail::hash_def_initval)
	{
		return hash_impl<hash_default_tag>::hash_word_array(
				  reinterpret_cast<uint32_t const*>(&value)
				, sizeof(ArithmeticT) / sizeof(uint32_t)
				, initval
			);
	}

	template<class ArithmeticT>
	inline
	typename boost::enable_if_c<
		  boost::is_arithmetic<ArithmeticT>::value && (sizeof(ArithmeticT) < sizeof(uint32_t))
		, hash_result_t
	>::type
	hash_object(ArithmeticT value, hash_result_t const initval = detail::hash_def_initval)
	{
		return hash_blob(&value, sizeof(value), initval);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct hash
	{
		typedef hash_result_t result_type;

		result_type operator()(T const& value) const
		{
			return hash_object(value);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__HASH_IMPL_HPP_

