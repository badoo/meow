///////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__NETDB_HPP_
#define MEOW_UNIX__NETDB_HPP_

#include <sys/types.h>
#include <netdb.h>

#include <cassert>
#include <cstdio>						// for snprintf

#include <meow/std_unique_ptr.hpp>
#include <meow/api_call_error.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct addrinfo os_addrinfo_t;

	struct addrinfo_deleter_t {
		void operator()(os_addrinfo_t *ai) { ::freeaddrinfo(ai); }
	};

	// addrinfo list handle, don't try to use with list elements
	typedef std::unique_ptr<os_addrinfo_t, addrinfo_deleter_t> os_addrinfo_list_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_UNIX_ADDRINFO_LIST_FOR_EACH(var, list) \
		for (os_addrinfo_t *var = list.get(); var; var = var -> ai_next)

#define MEOW_UNIX_ADDRINFO_LIST_P_FOR_EACH(var, list) \
		for (os_addrinfo_t *var = list; var; var = var -> ai_next)

///////////////////////////////////////////////////////////////////////////////////////////////

	inline size_t os_addrinfo_list_size(os_addrinfo_list_ptr const& al)
	{
		size_t result = 0;
		MEOW_UNIX_ADDRINFO_LIST_FOR_EACH(i, al) { ++result; }
		return result;
	}

	template<class T>
	T* os_addrinfo_addr(os_addrinfo_t const& ai)
	{
		assert(sizeof(T) == ai.ai_addrlen);
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

	// returns: the getaddrinfo() error code
	//       == 0 -> ok, ai_list outparameter is now filled with stuff
	//       != 0 -> error, call gai_strerror() to get error string
	inline int getaddrinfo_ex(
				  os_addrinfo_list_ptr *ai_list // outparameter
				, char const *host
				, char const *port
				, struct addrinfo const& hints
			)
	{
		assert(NULL != ai_list);

		struct addrinfo *result = NULL;
		int r = ::getaddrinfo(host, port, &hints, &result);

		if (0 == r)
		{
			assert(NULL != result); // success, so result cant be empty
			ai_list->reset(result);
		}

		return r;
	}


	// returns: the getaddrinfo() error code
	//       == 0 -> ok, ai_list outparameter is now filled with stuff
	//       != 0 -> error, call gai_strerror() to get error string
	inline int getaddrinfo_ex(
				  os_addrinfo_list_ptr *ai_list // outparameter
				, char const *host
				, char const *port
				, int proto_family
				, int socktype
				, int proto = 0
				, int flags = 0
			)
	{
		struct addrinfo hints = {};
		hints.ai_family = proto_family;
		hints.ai_socktype = socktype;
		hints.ai_protocol = proto;
		hints.ai_flags    = flags;

		if (host && '*' == *host)
			host = NULL;

		if (NULL == host)
			hints.ai_flags |= AI_PASSIVE;

		return getaddrinfo_ex(ai_list, host, port, hints);
	}

///////////////////////////////////////////////////////////////////////////////////////////////

	// returns: addrinfo call result, empty pointer if not found
	// throws: api_call_error_ex on real errors
	inline os_addrinfo_list_ptr getaddrinfo_ex(
			  char const *host
			, char const *port
			, struct addrinfo const& hints
			)
	{
		os_addrinfo_list_ptr ai_list;

		int r = getaddrinfo_ex(&ai_list, host, port, hints);
		if (0 != r)
			throw meow::api_call_error_ex<getaddrinfo_error_printer_t>(
					  r, "getaddrinfo_ex('%s', '%s', PF = %d, socktype = %d, proto = %d)"
					, host, port, hints.ai_family, hints.ai_socktype, hints.ai_protocol
				);

		return ai_list;
	}

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
		os_addrinfo_list_ptr ai_list;

		int r = getaddrinfo_ex(&ai_list, host, port, proto_family, socktype, proto);
		if (0 != r)
			throw meow::api_call_error_ex<getaddrinfo_error_printer_t>(
					  r, "getaddrinfo_ex('%s', '%s', PF = %d, socktype = %d, proto = %d)"
					, host, port, proto_family, socktype, proto
				);

		return ai_list;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	inline T getaddrinfo_first(
				  char const *host
				, char const *port
				, int proto_family
				, int socktype
				, int proto = 0
				)
	{
		os_addrinfo_list_ptr list = getaddrinfo_ex(host, port, proto_family, socktype, proto);
		return *os_addrinfo_addr<T>(*list);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__NETDB_HPP_

