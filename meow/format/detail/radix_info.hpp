////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_DETAIL__RADIX_INFO_HPP_
#define MEOW_FORMAT_DETAIL__RADIX_INFO_HPP_

#include <cstdlib>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<size_t N> struct radix_info_t;
	template<size_t N> struct radix_info_H_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct radix_info_t<2>
	{
		enum { radix = 2 };

		template<class CharT>
		static CharT get_character(CharT, ssize_t const val)
		{
			static CharT const chars_[] = {
				'1', '0', '1'
			};
			static size_t const chars_sz = sizeof(chars_) / sizeof(chars_[0]);

			ssize_t const off = val + 1;
			BOOST_ASSERT(off < chars_sz);
			return chars_[off];
		}
	};

	template<>
	struct radix_info_t<10>
	{
		enum { radix = 10 };

		template<class CharT>
		static CharT get_character(CharT, ssize_t const val)
		{
			static CharT const chars_[] = {
					   '9', '8', '7', '6', '5', '4', '3', '2', '1'
				, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
			};
			static size_t const chars_sz = sizeof(chars_) / sizeof(chars_[0]);

			ssize_t const off = val + 9;
			BOOST_ASSERT(off < chars_sz);
			return chars_[off];
		}
	};

	template<>
	struct radix_info_t<16>
	{
		enum { radix = 16 };

		template<class CharT>
		static CharT get_character(CharT, ssize_t const val)
		{
			static CharT const chars_[] = {
					   'f', 'e', 'd', 'c', 'b', 'a', '9', '8', '7', '6', '5', '4', '3', '2', '1'
				, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
			};
			static size_t const chars_sz = sizeof(chars_) / sizeof(chars_[0]);

			ssize_t const off = val + 15;
			BOOST_ASSERT(off < chars_sz);
			return chars_[off];
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct radix_info_H_t<16>
	{
		enum { radix = 16 };

		template<class CharT>
		static CharT get_character(CharT, ssize_t const val)
		{
			static CharT const chars_[] = {
					   'F', 'E', 'D', 'C', 'B', 'A', '9', '8', '7', '6', '5', '4', '3', '2', '1'
				, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
			};
			static size_t const chars_sz = sizeof(chars_) / sizeof(chars_[0]);

			ssize_t const off = val + 15;
			BOOST_ASSERT(off < chars_sz);
			return chars_[off];
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_DETAIL__RADIX_INFO_HPP_

