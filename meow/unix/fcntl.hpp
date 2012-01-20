////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__FCNTL_HPP_
#define MEOW_UNIX__FCNTL_HPP_

#include <fcntl.h>
#include <sys/ioctl.h>

#include <meow/api_call_error.hpp>
#include <meow/unix/fd_handle.hpp>
#include <meow/unix/libc_wrapper.hpp>

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
	#if defined(FIONBIO) // try using faster ioctl() interface if it's there
		int v = 1;
		return ::ioctl(s, FIONBIO, &v) ? -1 : s;
	#else
		int flags(::fcntl(s, F_GETFL, 0));
		return ::fcntl(s, F_SETFL, flags | O_NONBLOCK) ? -1 : s;
	#endif
	}

	inline int blocking(int s)
	{
	#if defined(FIONBIO) // try using faster ioctl() interface if it's there
		int v = 0;
		return ::ioctl(s, FIONBIO, &v) ? -1 : s;
	#else
		int flags(::fcntl(s, F_GETFL, 0));
		return ::fcntl(s, F_SETFL, flags & ~O_NONBLOCK) ? -1 : s;
	#endif
	}

	inline int close_on_exec(int s)
	{
		return ::fcntl(s, F_SETFD, FD_CLOEXEC) ? -1 : s;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline void set_socket_blocking(fd_handle_t& fd, bool is_blocking)
	{
		int r = (is_blocking)
				? blocking(get_handle(fd))
				: nonblocking(get_handle(fd))
				;

		if (-1 == r)
			throw meow::api_call_error("set_socket_blocking(%d, %s)", get_handle(fd), is_blocking ? "true" : "false");
	}

	inline void set_nonblocking(fd_handle_t& fd)
	{
		set_socket_blocking(fd, false);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__FCNTL_HPP_

