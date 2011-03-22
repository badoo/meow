////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_FWD_HPP_
#define MEOW_HASH__HASH_FWD_HPP_

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

	template<class ArithmeticT>
	struct hash<
		  ArithmeticT
		, typename boost::enable_if<boost::is_arithmetic<ArithmeticT> >::type
		>
	{
		typedef hash_result_t result_type;

		inline result_type operator()(ArithmeticT const& value) const
		{
			BOOST_STATIC_ASSERT(0 == (sizeof(ArithmeticT) % sizeof(uint32_t)));

			if (0 == (sizeof(ArithmeticT) % sizeof(uint32_t)))
			{
				return hash_impl<hash_default_tag>::hash_word_array(
						  reinterpret_cast<uint32_t const*>(&value)
						, sizeof(ArithmeticT) / sizeof(uint32_t)
						, detail::hash_def_initval
					);
			}
			else
			{
				return hash_impl<hash_default_tag>::hash_blob(
						  &value
						, sizeof(ArithmeticT)
						, detail::hash_def_initval
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
	{
		typedef hash_result_t result_type;

		inline result_type operator()(T v) const
		{
			// check if string is longer than 4 bytes
			static size_t const n_bytes = 4;
			char const *b = (char const*)v;
			char const *e = (char const*)memchr(b, 0x0, n_bytes);
			if (NULL == e) e = b + n_bytes;

			return hash_impl<hash_default_tag>::hash_blob(b, (e - b), detail::hash_def_initval);
		}
	};

	template<class T>
	struct hash<string_ref<T> >
	{
		typedef hash_result_t result_type;

		inline result_type operator()(string_ref<T> const& v) const
		{
			return hash_impl<hash_default_tag>::hash_blob(v.begin(), v.c_length() * sizeof(T), detail::hash_def_initval);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	// convenience, when you just have the object to hash at hand
	template<class T>
	inline hash_result_t hash_object(T const& obj)
	{
		return hash<T>()(obj);
	}

	// convenience, when you just need to hash some array as raw memory, useful for fixed length strings
	template<class T, std::size_t N>
	inline hash_result_t hash_object(T const (&data)[N])
	{
		return hash_object(ref_lit(data));
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__HASH_FWD_HPP_

