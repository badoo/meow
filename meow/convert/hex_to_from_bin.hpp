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

	inline char* bin2hex(unsigned char b, char* p)
	{
		unsigned char h = b >> 4;
		*p++ = '0' + h + (-(h > 9) & ('a' - '9' - 1));
		unsigned char l = b & 0xf;
		*p++ = '0' + l + (-(l > 9) & ('a' - '9' - 1));
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

