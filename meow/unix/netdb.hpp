///////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__NETDB_HPP_
#define MEOW_UNIX__NETDB_HPP_

#include <sys/types.h>
#include <netdb.h>

#include <cstdio>						// for snprintf
#include <boost/assert.hpp>

#include <meow/move_ptr/static_move_ptr.hpp>
#include <meow/api_call_error.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct addrinfo os_addrinfo_t;

	struct addrinfo_deleter_t {
		void operator()(os_addrinfo_t *ai) { ::freeaddrinfo(ai); }
	};

	// addrinfo list handle, don't try to use with list elements
	typedef boost::static_move_ptr<os_addrinfo_t, addrinfo_deleter_t> os_addrinfo_list_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_UNIX_ADDRINFO_LIST_FOR_EACH_EX(ai_ptr, curr_varname) \
	for (os_addrinfo_t * curr_varname = get_pointer(ai_ptr); curr_varname; curr_varname = curr_varname -> ai_next)

#define MEOW_UNIX_ADDRINFO_LIST_FOR_EACH(ai_ptr) MEOW_UNIX_ADDRINFO_LIST_FOR_EACH_EX(ai_ptr, curr)

///////////////////////////////////////////////////////////////////////////////////////////////

	inline size_t os_addrinfo_list_size(os_addrinfo_list_ptr const& al)
	{
		size_t result = 0;
		MEOW_UNIX_ADDRINFO_LIST_FOR_EACH_EX(al, curr_ai) { ++result; }
		return result;
	}

	template<class T>
	T* os_addrinfo_addr(os_addrinfo_t const& ai)
	{
		BOOST_ASSERT(sizeof(T) == ai.ai_addrlen);
		return reinterpret_cast<T*>(ai.ai_addr);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct getaddrinfo_error_printer_t
	{
		size_t operator()(int error_code, char* buf, int buf_len) const
		{
			return snprintf(buf, buf_len, " - %s", gai_strerror(error_code));
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////

	// if (NULL == host) -> assumes everything i.e. INADDR_ANY for IPv4
	// returns: addrinfo call result, empty pointer if not found
	// throws: api_call_error_ex on real errors
	inline os_addrinfo_list_ptr getaddrinfo_ex(
			  char const *host
			, char const *port
			, int proto_family
			, int socktype
			, int proto = 0
			)
	{
		struct addrinfo hints = {};
		hints.ai_family = proto_family;
		hints.ai_socktype = socktype;
		hints.ai_protocol = proto;

		if (host && '*' == *host)
			host = NULL;

		if (NULL == host)
			hints.ai_flags |= AI_PASSIVE;

		struct addrinfo *result = NULL;
		int r = ::getaddrinfo(host, port, &hints, &result);

		if (0 != r)
		{
#if 0
#ifdef EAI_NODATA
			if (EAI_NODATA == r)
				return os_addrinfo_list_ptr();
#endif
			if (EAI_NONAME == r)
				return os_addrinfo_list_ptr();
#endif // 0
			throw meow::api_call_error_ex<getaddrinfo_error_printer_t>(
					  r, "getaddrinfo_ex('%s', '%s', PF = %d, socktype = %d, proto = %d)"
					, host, port, proto_family, socktype, proto
				);
		}

		BOOST_ASSERT(NULL != result); // success, so result cant be empty
		return os_addrinfo_list_ptr(result);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__NETDB_HPP_

