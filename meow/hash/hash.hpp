////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2007+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_HPP_
#define MEOW_HASH_HPP_

#include <cstdlib>		// for std::size_t
#include <functional>	// for std::unary_function

#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <meow/str_ref.hpp>
#include <meow/hash/jenkins_hash.hpp>
//#include "integer_traits.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	enum { def_initval = 0x0badf00d };

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#if (MEOW_ARCH_LITTLE_ENDIAN == 1)
# 	define meow_hash_function_bytes hashlittle
#elif (MEOW_ARCH_BIG_ENDIAN == 1)
# 	define meow_hash_function_bytes hashbig
#else /* couldn't determine endianess - assume we're little endian for hash */
# 	define meow_hash_function_bytes hashlittle
#endif

	template<class T, class Enabler = void> struct hash; // no default impl

	template<class ArithmeticT>
	struct hash<ArithmeticT, typename boost::enable_if<boost::is_arithmetic<ArithmeticT> >::type>
		: public std::unary_function<ArithmeticT, std::size_t>
	{
		std::size_t operator()(ArithmeticT const& value) const
		{
			BOOST_STATIC_ASSERT(0 == (sizeof(ArithmeticT) % sizeof(uint32_t)));

			if (0 == (sizeof(ArithmeticT) % sizeof(uint32_t)))
			{
				return hash_fn::hashword(
						  reinterpret_cast<uint32_t const*>(&value)
						, sizeof(ArithmeticT) / sizeof(uint32_t)
						, detail::def_initval
					);
			}
			else
			{
				return hash_fn::meow_hash_function_bytes(
						  &value
						, sizeof(ArithmeticT)
						, detail::def_initval
					);
			}
		}
	};

	template<class T>
	struct hash<T, typename boost::enable_if_c<
			   boost::is_pointer<T>::value
			&& (boost::is_convertible<char*, T>::value
			  ||boost::is_convertible<signed char*, T>::value
			  ||boost::is_convertible<unsigned char*, T>::value
			)
		>::type>
		: public std::unary_function<T, std::size_t>
	{
		std::size_t operator()(T v) const
		{
			// check if string is longer than 4 bytes
			static size_t const n_bytes = 4;
			char const *b = (char const*)v;
			char const *e = (char const*)memchr(b, 0x0, n_bytes);
			if (NULL == e) e = b + n_bytes;

			return hash_fn::meow_hash_function_bytes(b, (e - b), detail::def_initval);
		}
	};

	template<class T>
	struct hash<string_ref<T> >
		: public std::unary_function<string_ref<T>, std::size_t>
	{
		std::size_t operator()(string_ref<T> const& v) const
		{
			return hash_fn::meow_hash_function_bytes(v.begin(), v.size() * sizeof(T), detail::def_initval);
		}
	};

#if defined(meow_hash_function_bytes)
# 	undef meow_hash_function_bytes
#endif

////////////////////////////////////////////////////////////////////////////////////////////////

	// convenience, when you just have the object to hash at hand
	template<class T>
	std::size_t hash_object(T const& obj)
	{
		return hash<T>()(obj);
	}

	// convenience, when you just need to hash some array as raw memory, useful for fixed length strings
	template<class T, std::size_t N>
	std::size_t hash_object(T const (&data)[N])
	{
		return hash_object(str_ref(data, N));
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__HASH_HPP_

