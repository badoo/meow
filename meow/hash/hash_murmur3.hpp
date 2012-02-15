////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_MURMUR3_HPP_
#define MEOW_HASH__HASH_MURMUR3_HPP_

#include <meow/hash/hash_fwd.hpp>
#include <meow/hash/murmur3.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct hash_murmur3_tag;

#ifndef MEOW_HASH_DEFAULT_TAG
#define MEOW_HASH_DEFAULT_TAG meow::hash_murmur3_tag
#endif

	template<>
	struct hash_impl<hash_murmur3_tag>
	{
		static hash_result_t hash_word_array(uint32_t const *p, unsigned const len_32, uint32_t const initval)
		{
			hash_result_t result;
			hash_fn::MurmurHash3(p, len_32 * 4, initval, &result);
			return result;
		}

		static hash_result_t hash_blob(void const *p, unsigned const len, uint32_t const initval)
		{
			hash_result_t result;
			hash_fn::MurmurHash3(p, len, initval, &result);
			return result;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/hash/hash_impl.hpp>

#endif // MEOW_HASH__HASH_MURMUR3_HPP_

