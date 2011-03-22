////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_MURMUR3_HPP_
#define MEOW_HASH__HASH_MURMUR3_HPP_

#include <meow/hash/hash_fwd.hpp>
#include <meow/hash/hash_impl.hpp>
#include <meow/hash/murmur3.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct hash_murmur3_tag;
	typedef hash_murmur3_tag hash_default_tag;

	template<>
	struct hash_impl<hash_murmur3_tag>
	{
		static hash_result_t hash_word_array(uint32_t *p, unsigned len_32, uint32_t initval)
		{
			return hash_fn::MurmurHash3_x64_32(p, len_32 * 4, initval);
		}

		static hash_result_t hash_blob(void *p, unsigned len, uint32_t initval)
		{
			return hash_fn::MurmurHash3_x64_32(p, len, initval);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__HASH_MURMUR3_HPP_

