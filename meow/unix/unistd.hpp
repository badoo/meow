////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__UNISTD_HPP_
#define MEOW_UNIX__UNISTD_HPP_

#ifndef _FILE_OFFSET_BITS
#	define _FILE_OFFSET_BITS 64 // looser
#endif

#include <sys/types.h>
#include <unistd.h>

#include "libc_wrapper.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, open, ((const char*, "path: %s")) ((int, "flags: 0x%08x")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, open, ((const char*, "path: %s")) ((int, "flags: 0x%08x")) ((mode_t, "mode: %04o")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, close, ((int, "fd: %d")));
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(ssize_t, read, ((int, "fd: %d")) ((void*, "%p")) ((size_t, "%zu")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(ssize_t, write, ((int, "fd: %d")) ((void const*, "%p")) ((size_t, "%zu")) );
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, pipe, ((int*, "%p")));

    MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, chdir, ((char const*, "%s")));

////////////////////////////////////////////////////////////////////////////////////////////////

	inline pid_t 		getpid() { return ::getpid(); }
	inline unsigned 	ugetpid() { return unsigned(::getpid()); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////


#endif // MEOW_UNIX_UNISTD_HPP_

