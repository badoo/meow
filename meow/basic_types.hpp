////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__BASIC_TYPES_HPP_
#define MEOW__BASIC_TYPES_HPP_

#include <inttypes.h>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct length_value
	{
		typedef uint64_t type;

		static type const unset  = type(-1);
		static type const max    = type(-2);
	};
	typedef length_value::type length_t;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__BASIC_TYPES_HPP_

