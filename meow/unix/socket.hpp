////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__SOCKET_HPP_
#define MEOW_UNIX__SOCKET_HPP_

#include <sys/types.h>
#include <sys/socket.h>

#include <meow/api_call_error.hpp>
#include <meow/unix/libc_wrapper.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

    typedef struct sockaddr         os_sockaddr_t;
    typedef struct sockaddr_in      os_sockaddr_in_t;
    typedef struct sockaddr_in6     os_sockaddr_in6_t;

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, socket, 	((int, "domain: %d"))
													((int, "type: %d"))
													((int, "proto: %d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, bind, 	((int, "sock: %d"))
													((os_sockaddr_t const*, "%p"))
													((socklen_t, "%d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, listen, 	((int, "sock: %d"))
													((int, "backlog: %d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, connect, ((int, "sock: %d"))
													((os_sockaddr_t const*, "%p"))
													((socklen_t, "%d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, accept, ((int, "sock: %d"))
													((os_sockaddr_t*, "%p"))
													((socklen_t*, "%p"))
													);

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	T getsockopt_ex(int fd, int level, int opt)
	{
		T result;
		socklen_t opt_len = sizeof(result);
		if (-1 == ::getsockopt(fd, level, opt, (void*)&result, &opt_len))
			throw meow::api_call_error("getsockopt(%d, %d, %d)", fd, level, opt);
		return result;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__SOCKET_HPP_
