////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
// Credits: some extra ideas borrowed from 
//  BOOST: boost/detail/endian.hpp
//  BOB JENKINS HASH: http://burtleburtle.net/bob/c/lookup3.c
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONFIG_ENDIAN_HPP_
#define MEOW_CONFIG_ENDIAN_HPP_

#include <sys/param.h>

// GNU libc offers <endian.h> which defines __BYTE_ORDER
#ifdef __GLIBC__
#	include <endian.h>
#endif

// macosx provides machine/endian.h which defines __DARWIN_BYTE_ORDER et. al.
#ifdef __APPLE__
#	include <machine/endian.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////

// glibc and other nice people
#if (defined(__BYTE_ORDER))
#	if (defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN)
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
#	endif
#	if (defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN)
#		define MEOW_ARCH_LITTLE_ENDIAN 0
#		define MEOW_ARCH_BIG_ENDIAN 1
#		define MEOW_ARCH_BYTE_ORDER 4321
#	endif
//#endif // defined(__BYTE_ORDER)

// apple has something special for us?
#elif (defined(__MACH__) || defined(__APPLE__))
#	if (defined(__DARWIN_BYTE_ORDER))
#		if (defined(__DARWIN_LITTLE_ENDIAN) && (__DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN))
#			define MEOW_ARCH_LITTLE_ENDIAN 1
#			define MEOW_ARCH_BIG_ENDIAN 0
#			define MEOW_ARCH_BYTE_ORDER 1234
#		endif
#		if (defined(__DARWIN_BIG_ENDIAN) && (__DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN))
#			define MEOW_ARCH_LITTLE_ENDIAN 0
#			define MEOW_ARCH_BIG_ENDIAN 1
#			define MEOW_ARCH_BYTE_ORDER 4321
#		endif
#	endif
//# endif // (defined(__MACH__) || defined(__APPLE__))

// any kind of intel? -> little endian
#elif  (defined(i386) || defined(__i386__) || defined(__i486__) || \
	defined(__i586__) || defined(__i686__))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // defined(__i*86__)

// some flavour of AMD64? -> little endian
#elif (defined(__amd64__) || defined(_M_AMD64) \
	|| defined(__x86_64) || defined(__x86_64__) \
	|| defined(_M_X64))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // amd64

// itanium -> little endian
#elif (defined(__ia64) || defined(__ia64__))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // itanium

// some other generic x86 chip -> little endian
#elif (defined(_M_IX86))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // generic x86

// alpha -> little endian
#elif (defined(__alpha__) || defined(_M_ALPHA))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // alpha

// vax and mips -> little endian
#elif (defined(vax) || defined(MIPSEL) || defined(_MIPSEB))
#		define MEOW_ARCH_LITTLE_ENDIAN 1
#		define MEOW_ARCH_BIG_ENDIAN 0
#		define MEOW_ARCH_BYTE_ORDER 1234
//#endif // vax & mips

// sparc, ppc, motorolla -> big endian
#elif (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
#		define MEOW_ARCH_LITTLE_ENDIAN 0
#		define MEOW_ARCH_BIG_ENDIAN 1
#		define MEOW_ARCH_BYTE_ORDER 4321
//#endif // sparc, ppc, motorolla

#else
#	error "CONFIG/ENDIAN.HPP: failed to guess endianes"
#endif // all endianess checks

////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONFIG_ENDIAN_HPP_

