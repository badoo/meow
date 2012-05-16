////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__RESOURCE_HPP_
#define MEOW_UNIX__RESOURCE_HPP_

#include <sys/types.h>
#include <sys/resource.h>

#ifdef linux
	#include <sys/prctl.h>
#endif

#include <meow/api_call_error.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct rusage os_rusage_t;

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline os_rusage_t getrusage_ex(int who)
	{
		os_rusage_t ru;
		if (-1 == ::getrusage(who, &ru))
			throw meow::api_call_error("getrusage(who: %d)", who);
		return ru;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool set_dumpable(bool is_enabled)
	{
#ifdef linux
		// NOTE: prctl has 5 args, we need only 2, 
		//       but valgrind et. al. complain that other args are uninitialized
		//       so we just set them to zeroes and don't give a shit anymore.
		// NOTE: this call is linux specific.
		// NOTE: this function should always return true, as there are no errors in syscall spec. see manual.
		return 0 == ::prctl(PR_SET_DUMPABLE, is_enabled ? 1 : 0, 0, 0, 0);
#else
		return true;
#endif
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	namespace detail {
		// NOTE: can't implement it as something like 
		//   template<int resource> struct rlimit_name_t { static char const name[]; };
		//   and a set of specializations
		//   because gcc ver >= 4 complains
		//   error: initializer-string for array of chars is too long
		//   a bug has been opened for this issue: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=27347

		#define RLIMIT_NAME(name) "setrlimit(" name ")"
		#define RLIMIT_FULL_NAME(resource, name) RLIMIT_NAME(#resource ", " name)

		// primary template function
		template<int resource> inline char const* rlimit_name() { return RLIMIT_NAME("unknown resource"); }

		// explicit full specialization maker macro
#if 0
		#define DEFINE_RLIMIT_NAME(resource, name)		\
			template<> inline char const* rlimit_name<resource>() { return RLIMIT_FULL_NAME(resource, name); }
#endif
		#define DEFINE_RLIMIT_NAME(resource, name) 	\
			template<> inline char const* rlimit_name<resource>() { return name; }

#ifdef linux
		DEFINE_RLIMIT_NAME(RLIMIT_AS, "address space size, bytes");
#endif

		DEFINE_RLIMIT_NAME(RLIMIT_CORE, "core file size");
		DEFINE_RLIMIT_NAME(RLIMIT_CPU, "cpu time, seconds");
		DEFINE_RLIMIT_NAME(RLIMIT_DATA, "data segment size");
		DEFINE_RLIMIT_NAME(RLIMIT_FSIZE, "file size");

#ifdef linux
		// man setrlimit: RLIMIT_LOCKS, 'Early Linux 2.4 only'
		DEFINE_RLIMIT_NAME(RLIMIT_LOCKS, "number of flock() and fcntl() leases");
#endif
		// man setrlimit: RLIMIT_MEMLOCK  and  RLIMIT_NPROC  derive  from  BSD  
		//     and  are  not  specified in POSIX.1-2001; 
		//     they are present on the BSDs and Linux, 
		//     but on few other implementations.
		DEFINE_RLIMIT_NAME(RLIMIT_MEMLOCK, "number of bytes for mlock()/mlockall()");
		DEFINE_RLIMIT_NAME(RLIMIT_NPROC, "number of child processes");
		//
		DEFINE_RLIMIT_NAME(RLIMIT_NOFILE, "number of file descriptors");
		// man setrlimit: RLIMIT_RSS derives from BSD and is not specified in POSIX.1-2001;
		//     it is nevertheless present on most implementations.
		DEFINE_RLIMIT_NAME(RLIMIT_RSS, "resident set size, pages");
		DEFINE_RLIMIT_NAME(RLIMIT_STACK, "stack size, bytes");

		#ifdef RLIMIT_MSGQUEUE
		// man setrlimit: this call is LINUX SPECIFIC, 'Since Linux 2.6.8'
		DEFINE_RLIMIT_NAME(RLIMIT_MSGQUEUE, "mq_open() messages size, bytes, per UID");
		#endif

		#ifdef RLIMIT_SIGPENDING
		// man setrlimit: this call is LINUX SPECIFIC, 'Since Linux 2.6.8'
		DEFINE_RLIMIT_NAME(RLIMIT_SIGPENDING, "sigqueue(), max pending signals, per UID");
		#endif

		// DEFINE_RLIMIT_NAME(RLIMIT_, "");

		#undef DEFINE_RLIMIT_NAME
		#undef RLIMIT_NAME

	} // namespace detail

	typedef struct rlimit os_rlimit_t;

	template<int resource>
	inline void setrlimit_ex(os_rlimit_t const& rl)
	{
		if (-1 == ::setrlimit(resource, &rl))
		{
			throw meow::api_call_error(
					  "setrlimit_ex(%d : '%s', { soft: %zd, hard: %zd })"
					, resource, detail::rlimit_name<resource>()
					, static_cast<ssize_t>(rl.rlim_cur), static_cast<ssize_t>(rl.rlim_max)
				);
		}
	}

	template<int resource>
	inline os_rlimit_t getrlimit_ex()
	{
		os_rlimit_t result;
		if (-1 == ::getrlimit(resource, &result))
		{
			throw meow::api_call_error(
					  "getrlimit_ex(%d : '%s')"
					, resource, detail::rlimit_name<resource>()
				);
		}
		return result;
	}

	inline os_rlimit_t rlimit_cur_max(rlim_t cur, rlim_t max) { os_rlimit_t const r = { cur, max }; return r; }
	inline os_rlimit_t rlimit_both(rlim_t limit) { return rlimit_cur_max(limit, limit); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__RESOURCE_HPP_

