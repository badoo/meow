////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2018 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONVERT__BASE64_HPP_
#define MEOW_CONVERT__BASE64_HPP_

#include <string>   // std::string
#include <cstring>  // memcpy

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct base64_tables_t {
		unsigned char encode[64];
		unsigned char decode[256];
		unsigned char padding;
	};

	inline base64_tables_t const& base64_default_tables() noexcept
	{
		static base64_tables_t const tables_ = {
			.encode = {
				'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
				'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
			},
			.decode =  {
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
			    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
			    77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
			    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
			    77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			},
			.padding = '='
		};

		return tables_;
	}

	inline constexpr size_t base64_encoded_len(size_t len) noexcept
	{
		return (((len + 2) / 3) * 4);
	}

	inline constexpr size_t base64_decoded_len(size_t len) noexcept
	{
		return (((len + 3) / 4) * 3);
	}

	inline base64_tables_t base64_tables_build(char const basis[65]) noexcept
	{
		base64_tables_t tables;

		memcpy(tables.encode, basis, 64);

		for (size_t i = 0; i < sizeof(tables.decode); ++i)
			tables.decode[i] = 77;
		for (size_t i = 0; i < 64; ++i)
			tables.decode[(unsigned)basis[i]] = (unsigned char)i;

		tables.padding = basis[64];

		return tables;
	}

	inline size_t base64_encode_ex(char *dst, char const *src, size_t src_len, base64_tables_t const& tables) noexcept
	{
		char                *d = dst;
		unsigned char const *s = (unsigned char const*)src;
		unsigned char const *basis64 = tables.encode;

		while (src_len > 2) {
		    *d++ = basis64[(s[0] >> 2) & 0x3f];
		    *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
		    *d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		    *d++ = basis64[s[2] & 0x3f];

		    s += 3;
		    src_len -= 3;
		}

		if (src_len) {
		    *d++ = basis64[(s[0] >> 2) & 0x3f];

		    if (src_len == 1) {
		        *d++ = basis64[(s[0] & 3) << 4];
		        *d++ = tables.padding;

		    } else {
		        *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
		        *d++ = basis64[(s[1] & 0x0f) << 2];
		    }

		    *d++ = tables.padding;
		}

		return (d - dst);
	}

	inline size_t base64_decode_ex(char *dst, char const *src, size_t src_len, base64_tables_t const& tables) noexcept
	{
		size_t               len;
		char                *d = dst;
		unsigned char const *s = (unsigned char const *)src;
		unsigned char const *basis = tables.decode;

		for (len = 0; len < src_len; len++) {

			if (s[len] == tables.padding) {
		        break;
		    }

		    if (basis[s[len]] == 77) {
		        return 0;
		    }
		}

		if (len % 4 == 1) {
		    return 0;
		}

		while (len > 3) {
		    *d++ = (char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
		    *d++ = (char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
		    *d++ = (char) (basis[s[2]] << 6 | basis[s[3]]);

		    s += 4;
		    len -= 4;
		}

		if (len > 1) {
		    *d++ = (char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
		}

		if (len > 2) {
		    *d++ = (char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
		}

		return (d - dst);
	}

	inline std::string base64_encode(str_ref s, base64_tables_t const& tables = base64_default_tables())
	{
		std::string result ( base64_encoded_len(s.size()), '\0' );

		size_t const real_sz = base64_encode_ex(&result[0], s.data(), s.c_length(), tables);
		result.resize(real_sz);

		return result;
	}

	inline std::string base64_decode(str_ref s, base64_tables_t const& tables = base64_default_tables())
	{
		std::string result ( base64_decoded_len(s.size()), '\0' );

		size_t const real_sz = base64_decode_ex(&result[0], s.data(), s.c_length(), tables);
		result.resize(real_sz);

		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONVERT__BASE64_HPP_

