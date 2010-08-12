////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__SYS_MMAN_HPP_
#define MEOW_UNIX__SYS_MMAN_HPP_

#include <sys/types.h>
#include <sys/mman.h>

#include "libc_wrapper.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, mlock, ((void const*, "%p")) ((size_t, "%zu")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, munlock, ((void const*, "%p")) ((size_t, "%zu")) );

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, mlockall, ((int, "flags: 0x%08x")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, munlockall);

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////


#endif // MEOW_UNIX__SYS_MMAN_HPP_

