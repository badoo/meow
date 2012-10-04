////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
// (c) 2005 based on code by Maxim Egorushkin
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONVERT_HEX_TO_FROM_BIN_HPP_
#define MEOW_CONVERT_HEX_TO_FROM_BIN_HPP_

#include <stdint.h>
#include <boost/assert.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool is_xdigit(unsigned char c)
	{
		return ('0' <= c && c <= '9') || ('a' <= (c | 0x20) && (c | 0x20) <= 'f');
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline uint8_t hex2bin(unsigned char h, unsigned char l)
	{
		h |= 0x20;
		h -= 0x30;
		h -= -(h > 9) & 0x27;
		l |= 0x20;
		l -= 0x30;
		l -= -(l > 9) & 0x27;
		return h << 4 | l;
	}

	// this version seems to be slower than the table-based one
	//  by a factor of about 1.8x on all current cpus (core2+)
	inline char* bin2hex_pure_code(unsigned char b, char* p)
	{
		unsigned char h = b >> 4;
		*p++ = '0' + h + (-(h > 9) & ('a' - '9' - 1));
		unsigned char l = b & 0xf;
		*p++ = '0' + l + (-(l > 9) & ('a' - '9' - 1));
		return p;
	}

	inline char* bin2hex(unsigned char b, char* p)
	{
		static char const codes_2[] = {
			'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7',
			'0', '8', '0', '9', '0', 'a', '0', 'b', '0', 'c', '0', 'd', '0', 'e', '0', 'f',
			'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7',
			'1', '8', '1', '9', '1', 'a', '1', 'b', '1', 'c', '1', 'd', '1', 'e', '1', 'f',
			'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
			'2', '8', '2', '9', '2', 'a', '2', 'b', '2', 'c', '2', 'd', '2', 'e', '2', 'f',
			'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7',
			'3', '8', '3', '9', '3', 'a', '3', 'b', '3', 'c', '3', 'd', '3', 'e', '3', 'f',
			'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7',
			'4', '8', '4', '9', '4', 'a', '4', 'b', '4', 'c', '4', 'd', '4', 'e', '4', 'f',
			'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7',
			'5', '8', '5', '9', '5', 'a', '5', 'b', '5', 'c', '5', 'd', '5', 'e', '5', 'f',
			'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7',
			'6', '8', '6', '9', '6', 'a', '6', 'b', '6', 'c', '6', 'd', '6', 'e', '6', 'f',
			'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
			'7', '8', '7', '9', '7', 'a', '7', 'b', '7', 'c', '7', 'd', '7', 'e', '7', 'f',
			'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7',
			'8', '8', '8', '9', '8', 'a', '8', 'b', '8', 'c', '8', 'd', '8', 'e', '8', 'f',
			'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7',
			'9', '8', '9', '9', '9', 'a', '9', 'b', '9', 'c', '9', 'd', '9', 'e', '9', 'f',
			'a', '0', 'a', '1', 'a', '2', 'a', '3', 'a', '4', 'a', '5', 'a', '6', 'a', '7',
			'a', '8', 'a', '9', 'a', 'a', 'a', 'b', 'a', 'c', 'a', 'd', 'a', 'e', 'a', 'f',
			'b', '0', 'b', '1', 'b', '2', 'b', '3', 'b', '4', 'b', '5', 'b', '6', 'b', '7',
			'b', '8', 'b', '9', 'b', 'a', 'b', 'b', 'b', 'c', 'b', 'd', 'b', 'e', 'b', 'f',
			'c', '0', 'c', '1', 'c', '2', 'c', '3', 'c', '4', 'c', '5', 'c', '6', 'c', '7',
			'c', '8', 'c', '9', 'c', 'a', 'c', 'b', 'c', 'c', 'c', 'd', 'c', 'e', 'c', 'f',
			'd', '0', 'd', '1', 'd', '2', 'd', '3', 'd', '4', 'd', '5', 'd', '6', 'd', '7',
			'd', '8', 'd', '9', 'd', 'a', 'd', 'b', 'd', 'c', 'd', 'd', 'd', 'e', 'd', 'f',
			'e', '0', 'e', '1', 'e', '2', 'e', '3', 'e', '4', 'e', '5', 'e', '6', 'e', '7',
			'e', '8', 'e', '9', 'e', 'a', 'e', 'b', 'e', 'c', 'e', 'd', 'e', 'e', 'e', 'f',
			'f', '0', 'f', '1', 'f', '2', 'f', '3', 'f', '4', 'f', '5', 'f', '6', 'f', '7',
			'f', '8', 'f', '9', 'f', 'a', 'f', 'b', 'f', 'c', 'f', 'd', 'f', 'e', 'f', 'f'
		};
		static uint16_t const *real_codes = reinterpret_cast<uint16_t const*>(codes_2);

		memcpy(p, real_codes + b, sizeof(uint16_t));
		p += sizeof(uint16_t);

		return p;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// AnToXa: i'm too lazy to write custom iterator (an adapter really)
//  and use std::transform() with it

	// length must be sufficient
	//  returns to + (e - b) / 2
	inline char* copy_hex2bin(char const *b, char const *e, char *to)
	{
		BOOST_ASSERT((0 == ((e - b) % 2)) && "length of incoming hex-string must be even number");
		for (; b != e; b += 2, ++to)
			*to = hex2bin(b[0], b[1]);
		return to;
	}

	// length must be sufficient
	//  returns to + 2 * (e - b)
	inline char* copy_bin2hex(char const *b, char const *e, char *to)
	{
		for (; b != e; ++b)
			to = bin2hex(*b, to);
		return to;
	}

	// length must be sufficient
	//  returns to + 4 * (e - b)
	inline char* copy_bin2hex_escaped(char const *b, char const *e, char *to)
	{
		for (; b != e; ++b)
		{
			*to++ = '\\';
			*to++ = 'x';
			to = bin2hex(*b, to);
		}
		return to;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONVERT_HEX_TO_FROM_BIN_HPP_

