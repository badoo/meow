////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__CHAR_POINTER_HPP_
#define MEOW_FORMAT_INSERTER__CHAR_POINTER_HPP_

#include <meow/str_ref.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(type)	\
	template<> struct type_tunnel<type> {				\
		static str_ref call(type v) {					\
			return str_ref((char const*)v);				\
		}												\
	};													\
/**/

	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(char*);
	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(char const*);
	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(signed char*);
	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(signed char const*);
	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(unsigned char*);
	MEOW_FORMAT_DEFINE_CHAR_POINTER_TUNNEL(unsigned char const*);

#define MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(type, has_const)		\
	template<size_t N> struct type_tunnel<type has_const [N]> {		\
		static str_ref call(type const (&a) [N]) {					\
			return str_ref((char const*)a, N - 1);					\
		}															\
	};																\
/**/

	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(char, );
	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(char, const);
	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(signed char, );
	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(signed char, const);
	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(unsigned char, );
	MEOW_FORMAT_DEFINE_CHAR_ARRAY_TUNNEL(unsigned char, const);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__CHAR_POINTER_HPP_

