////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__FCNTL_HPP_
#define MEOW_UNIX__FCNTL_HPP_

#include <fcntl.h>
#include "libc_wrapper.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: add more wrapper variants as required here

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, fcntl, 	((int, "fd: %d"))
													((int, "cmd: %d"))
													((int, "arg0: %d"))
													);

////////////////////////////////////////////////////////////////////////////////////////////////

	inline int nonblocking(int s)
	{
		int flags(::fcntl(s, F_GETFL, 0));
		return ::fcntl(s, F_SETFL, flags | O_NONBLOCK) ? -1 : s;
	}

	inline int blocking(int s)
	{
		int flags(::fcntl(s, F_GETFL, 0));
		return ::fcntl(s, F_SETFL, flags & ~O_NONBLOCK) ? -1 : s;
	}

	inline int close_on_exec(int s)
	{
		return ::fcntl(s, F_SETFD, FD_CLOEXEC) ? -1 : s;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__FCNTL_HPP_

