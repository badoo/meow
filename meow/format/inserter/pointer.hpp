////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__POINTER_HPP_
#define MEOW_FORMAT_INSERTER__POINTER_HPP_

#include <meow/tmp_buffer.hpp>

#include <meow/convert/hex_to_from_bin.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	// all pointers printing
	template<class T>
	struct type_tunnel<T*>
	{
		enum { buffer_size = sizeof(T*) * 2 + 2 /* the leading "0x" */ };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(T const *v, buffer_t const& buf = buffer_t())
		{
			char const *b = (char*)&v;
			char *ee = buf.get();
			*ee++ = '0';
			*ee++ = 'x';
			ee = copy_bin2hex(b, b + sizeof(T*), ee);

			BOOST_ASSERT(ee <= buf.end());

			return str_ref(buf.get(), ee);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__POINTER_HPP_

