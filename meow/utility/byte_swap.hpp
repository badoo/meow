////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__BYTE_SWAP_HPP_
#define MEOW_UTILITY__BYTE_SWAP_HPP_

#include <inttypes.h>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_BYTE_SWAP_USE_GCC_BUILTINS
	#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4)
		#define MEOW_BYTE_SWAP_USE_GCC_BUILTINS 1
	#else
		#define MEOW_BYTE_SWAP_USE_GCC_BUILTINS 0
	#endif
#endif

	inline uint32_t byte_swap_32(uint32_t const v)
	{
	#if MEOW_BYTE_SWAP_USE_GCC_BUILTINS
		return __builtin_bswap32(v);
	#else
		return ((((uint32_t)(v) & 0xff000000) >> 24) |
				(((uint32_t)(v) & 0x00ff0000) >>  8) |
				(((uint32_t)(v) & 0x0000ff00) <<  8) |
				(((uint32_t)(v) & 0x000000ff) << 24));
	#endif
	}

	inline uint64_t byte_swap_64(uint64_t const v)
	{
	#if MEOW_BYTE_SWAP_USE_GCC_BUILTINS
		return __builtin_bswap64(v);
	#else
		return ((((uint64_t)(v) & 0xff00000000000000ull) >> 56) |
				(((uint64_t)(v) & 0x00ff000000000000ull) >> 40) |
				(((uint64_t)(v) & 0x0000ff0000000000ull) >> 24) |
				(((uint64_t)(v) & 0x000000ff00000000ull) >> 8 ) |
				(((uint64_t)(v) & 0x00000000ff000000ull) << 8 ) |
				(((uint64_t)(v) & 0x0000000000ff0000ull) << 24) |
				(((uint64_t)(v) & 0x000000000000ff00ull) << 40) |
				(((uint64_t)(v) & 0x00000000000000ffull) << 56));
	#endif
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__BYTE_SWAP_HPP_

