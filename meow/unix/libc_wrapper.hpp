////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX_LIBC_WRAPPER_HPP_
#define MEOW_UNIX_LIBC_WRAPPER_HPP_

/*
DESCRIPTION:
	define a function that wraps a function that
	returns -1 on error and >=0 on success,
	(most unix libc functions have this semantics)

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(result_type, syscall_name, parameter_sequence)
	where:
	 * result_type - syscall result type, must be covertible to int
	 * syscall_name - name of the system call, ex: open, stat, fstat, mmap
	 * parameter_sequence - a sequence of tuples, where every tuple has two elements:
							1. parameter type
							2. parameter printf format
		ex: ((char const*, "'%s'")) ((int, "0x%08x")) means that:
		 the first parameter has type: char const* and can be printf-ed as '%s'
		 the second parameter has type: int and format "0x%08x"

USAGE EXAMPLE:
	defining throwing wrapper for open(2) that has two forms:

	int open(const char *pathname, int flags);
	int open(const char *pathname, int flags, mode_t mode);

we write to macros:

// two parameter version
MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, open, ((char const*, "'%s'")) ((int, "0x%08x")) )
// three parameter version
MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, open, ((char const*, "'%s'")) ((int, "0x%08x")) ((mode_t, "0x%04x")) )

and get two overloaded inline functions:
inline int open_ex(char const*, int) throw(meow::api_call_error);
inline int open_ex(char const*, int, mode_t) throw(meow::api_call_error);

that look like this
inline int open_ex(char const* arg0, int arg1) throw(meow::api_call_error)
{
	int res = ::open(arg0, arg1);
	if (-1 == res)
		throw meow::api_call_error("open_ex('%s', 0x%08x)", arg0, arg1);
	return res;
}

functions without parameters are also supported, like this:
MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, inotify_init)

*/

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/less.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>

#include <meow/api_call_error.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
// this is private and should not be used directly
//  use MEOW_DEFINE_LIBC_THROWING_WRAPPER

#define MEOW_LIBCWRAP_SELECT_ELT(r, data, val) BOOST_PP_TUPLE_ELEM(2, data, val)

#define MEOW_LIBCWRAP_ENUM_ARGS_OP(z, n, data) BOOST_PP_SEQ_ELEM(n, data) BOOST_PP_CAT(arg, n)

#define MEOW_LIBCWRAP_ENUM_ARGS(seq_size, seq) \
	BOOST_PP_ENUM(seq_size, MEOW_LIBCWRAP_ENUM_ARGS_OP, seq)

// don't print ", " when on last param
#define MEOW_LIBCWRAP_FORMAT_STRING_OP_I(last_elem_n, n, val)				\
	val BOOST_PP_IIF(BOOST_PP_EQUAL(n, last_elem_n), BOOST_PP_EMPTY(), ", ")	\
/**/

// don't print anything at all after last param
#define MEOW_LIBCWRAP_FORMAT_STRING_OP(r, last_elem_n, n, val)			\
	BOOST_PP_IIF(															\
			  BOOST_PP_LESS(last_elem_n, n)									\
			, BOOST_PP_EMPTY()												\
			, MEOW_LIBCWRAP_FORMAT_STRING_OP_I(last_elem_n, n, val))		\
/**/

#define MEOW_LIBCWRAP_FORMAT_STRING(seq_size, seq)	\
	BOOST_PP_IF(seq_size								\
		, BOOST_PP_SEQ_FOR_EACH_I(						\
			  MEOW_LIBCWRAP_FORMAT_STRING_OP			\
			, BOOST_PP_DEC(seq_size)					\
			, seq)										\
		, BOOST_PP_EMPTY())								\
/**/

#define MEOW_DEFINE_LIBC_THROWING_WRAPPER_IMPL(res_t, syscall_name, n, types, formats)	\
	inline res_t BOOST_PP_CAT(syscall_name, _ex)									\
		(MEOW_LIBCWRAP_ENUM_ARGS(n, types))											\
	{																				\
		res_t res = ::syscall_name(BOOST_PP_ENUM_PARAMS(n, arg));					\
		if (-1 == res)																\
		{																			\
			throw meow::api_call_error(												\
					BOOST_PP_STRINGIZE(syscall_name)								\
					"("																\
						MEOW_LIBCWRAP_FORMAT_STRING(n, formats)						\
					")"																\
					BOOST_PP_ENUM_TRAILING_PARAMS(n, arg)							\
				);																	\
		}																			\
		return res;																	\
	}																				\
/* ENDMACRO: MEOW_DEFINE_LIBC_THROWING_WRAPPER_IMPL */

////////////////////////////////////////////////////////////////////////////////////////////////
// this is the public interface
// params is vararg to allow 0 or 1 parameter
// FIXME: use standard interface with __VA_ARGS__!

#define MEOW_DEFINE_LIBC_THROWING_WRAPPER_I(r, call, n, p)		\
	MEOW_DEFINE_LIBC_THROWING_WRAPPER_IMPL(r, call				\
		, n															\
		, BOOST_PP_SEQ_TRANSFORM(MEOW_LIBCWRAP_SELECT_ELT, 0, p)	\
		, BOOST_PP_SEQ_TRANSFORM(MEOW_LIBCWRAP_SELECT_ELT, 1, p)	\
	)																\
/* ENDMACRO: MEOW_DEFINE_LIBC_THROWING_WRAPPER_I */

#define MEOW_DEFINE_LIBC_THROWING_WRAPPER(res_t, syscall_name, params...)	\
	MEOW_DEFINE_LIBC_THROWING_WRAPPER_I(										\
			  res_t, syscall_name												\
			, BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(params ()))						\
			, params ()															\
		)																		\
/* ENDMACRO: MEOW_DEFINE_LIBC_THROWING_WRAPPER */

////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX_LIBC_WRAPPER_HPP_

