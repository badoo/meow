////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__SIGNAL_HPP_
#define MEOW_UNIX__SIGNAL_HPP_

#include <cassert>
#include <csignal>

#include <boost/noncopyable.hpp> 

#include "libc_wrapper.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __linux__
	extern const char *const sys_sigabbrev[];
#else
	extern const char *const sys_signame[];
#endif

extern const char *const sys_siglist[];

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline char const* signal_name_from_id(int sig)
	{
		assert(sig > 0 && sig < NSIG);
#ifdef __linux__
		return ::sys_sigabbrev[sig];
#else
		return ::sys_signame[sig];
#endif
	}

	inline char const* signal_desc_from_id(int sig)
	{
		assert(sig > 0 && sig < NSIG);
		return ::sys_siglist[sig];
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct sigaction	os_sigaction_t;
	typedef siginfo_t			os_siginfo_t;

	typedef void (*os_signal_fn_t)(int);
	typedef void (*os_sigaction_fn_t)(int, os_siginfo_t*, void*);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, sigaction,	((int, "%d"))
														((os_sigaction_t const*, "%p"))
														((os_sigaction_t*, "%p"))
														);

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__SIGNAL_HPP_

