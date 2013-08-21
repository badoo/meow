////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__SHA1_HPP_
#define MEOW_HASH__SHA1_HPP_

// adapted from redis: http://redis.io
// which borrowed it from Steve Reid <steve@edmweb.com>
//  original comments are still in the code sometimes where it improves clarity

#include <cstdint>
#include <meow/str_ref.hpp> // for convenience funcs

// implementation only
#include <cstring> 		// memcpy, memset
#include <meow/config/endian.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct sha1_digest_t
	{
		unsigned char data[20];
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct sha1_context_t
	{
		typedef sha1_context_t self_t;

		uint64_t count;
		uint32_t state[5];
		unsigned char buffer[64]; // 512 bit encode block

		inline sha1_context_t()
		{
			this->init();
		}

		// reset the state to initial
		void init()
		{
			// SHA1 initialization constants
			state[0] = 0x67452301;
			state[1] = 0xEFCDAB89;
			state[2] = 0x98BADCFE;
			state[3] = 0x10325476;
			state[4] = 0xC3D2E1F0;
			count = 0;
		}

		void reset()
		{
			this->init();
			std::memset(buffer, 0, sizeof(buffer));
		}

		// update with the new data block
		void update(unsigned char const *data, uint32_t len)
		{
			uint64_t i;
			uint64_t j = count;
			count += len << 3;

			j = (j >> 3) & 63;
			if ((j + len) > 63)
			{
				std::memcpy(&buffer[j], data, (i = 64 - j));
				self_t::transform(state, buffer);

				for (/**/; i + 63 < len; i += 64)
					self_t::transform(state, &data[i]);

				j = 0;
			}
			else
			{
				i = 0;
			}

			std::memcpy(&buffer[j], &data[i], len - i);
		}

		// dump the digest for current context
		//  WARNING: doesn't reset internal state, BUT leaves it in 'finalized' state, so you can't update it anymore
		//  (lil stupid that there is no easy way to get 'in progress' digests without copying)
		sha1_digest_t finalize_and_get_digest()
		{
			unsigned char finalcount[8];
			for (unsigned i = 0; i < 8; ++i)
				finalcount[i] = (unsigned char)((count >> ((8 - i - 1) * 8)) & 0xff);

			unsigned char c = 0200;
			this->update(&c, 1);

			while ((count & 504) != 448)
			{
				c = 0000;
				this->update(&c, 1);
			}
			this->update(finalcount, sizeof(finalcount));  /* Should cause a transform() */

			// expecting NRVO to happen here
			sha1_digest_t result;
			for (unsigned i = 0; i < 20; i++)
			{
				result.data[i] = (unsigned char)((state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
			}

			/* Wipe variables */
			std::memset(&finalcount, '\0', sizeof(finalcount));
			std::memset(&buffer, '\0', sizeof(buffer));

			return result;
		}

	private:

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
// this is defined in terms of block from transform() call

#if (1 == MEOW_ARCH_LITTLE_ENDIAN)
		#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) |(rol(block->l[i],8)&0x00FF00FF))
#elif (1 == MEOW_ARCH_BIG_ENDIAN)
		#define blk0(i) block->l[i]
#else
		#error "Endianness not defined!"
#endif


#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

		static void transform(uint32_t state[5], uint8_t const buf[64])
		{
			uint32_t a, b, c, d, e;

			union u_block_t
			{
				uint8_t 	c[64];
				uint32_t 	l[16];
			};

			u_block_t block[1]; /* use array to appear as a pointer */
			std::memcpy(block, buf, 64);

			/* Copy context->state[] to working vars */
			a = state[0];
			b = state[1];
			c = state[2];
			d = state[3];
			e = state[4];
			/* 4 rounds of 20 operations each. Loop unrolled. */
			R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
			R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
			R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
			R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
			R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
			R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
			R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
			R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
			R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
			R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
			R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
			R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
			R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
			R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
			R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
			R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
			R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
			R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
			R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
			R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
			/* Add the working vars back into context.state[] */
			state[0] += a;
			state[1] += b;
			state[2] += c;
			state[3] += d;
			state[4] += e;

			/* Wipe variables, i guess for 'security' reasons */
			a = b = c = d = e = 0;
			std::memset(block, '\0', sizeof(block));
		}

#undef R0
#undef R1
#undef R2
#undef R3
#undef R4
#undef blk
#undef blk0
#undef rol

	};

////////////////////////////////////////////////////////////////////////////////////////////////

	sha1_digest_t sha1_digest(char const *data, uint32_t length)
	{
		sha1_context_t ctx;
		ctx.update((unsigned char const*)data, length);
		return ctx.finalize_and_get_digest();
	}

	sha1_digest_t sha1_digest(str_ref const& s)
	{
		return sha1_digest(s.data(), s.c_length());
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/format/metafunctions.hpp>
#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/tmp_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct type_tunnel<sha1_digest_t>
	{
		typedef meow::tmp_buffer<sizeof(sha1_digest_t) * 2> buffer_t;

		static str_ref call(sha1_digest_t const& d, buffer_t const& buf = buffer_t())
		{
			char const *b = (char const*)d.data;
			char const *ee = copy_bin2hex(b, b + sizeof(d.data), buf.get());
			assert(ee == &*buf.end());

			return str_ref(buf.begin(), buf.end());
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__SHA1_HPP_

