////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__SOCKADDR_HPP_
#define MEOW_FORMAT_INSERTER__SOCKADDR_HPP_

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstring>
#include <cstdio>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/config/endian.hpp>
#include <meow/format/metafunctions.hpp>
#include <meow/format/inserter/integral.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	enum
	{
		port_strlen = sizeof(".ppppp")
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	inline str_ref os_sockaddr_print_to(buffer_ref buf, struct sockaddr_in const *sa, bool print_port = true)
	{
		#if (MEOW_ARCH_BYTE_ORDER == 1234)
			enum { _0, _1, _2, _3 };
		#else
			enum { _3, _2, _1, _0 };
		#endif

		uint8_t const* b = reinterpret_cast<uint8_t const*>(&sa->sin_addr.s_addr);

		char *begin = buf.begin();
		char *p = buf.end();

		if (print_port)
		{
			assert(buf.size() >= INET_ADDRSTRLEN + port_strlen);
			p = detail::integer_to_string(begin, p - begin, ntohs(sa->sin_port)); *--p = ':';
		}
		else
		{
			assert(buf.size() >= INET_ADDRSTRLEN);
		}

		p = detail::integer_to_string(begin, p - begin, b[_3]); *--p = '.';
		p = detail::integer_to_string(begin, p - begin, b[_2]); *--p = '.';
		p = detail::integer_to_string(begin, p - begin, b[_1]); *--p = '.';
		p = detail::integer_to_string(begin, p - begin, b[_0]);
		return str_ref(p, buf.end());
	}

	inline str_ref os_sockaddr_print_to(buffer_ref buf, struct sockaddr_in6 const *sa, bool print_port = true)
	{
		if (print_port)
		{
			assert(buf.size() >= INET6_ADDRSTRLEN + port_strlen);
		}
		else
		{
			assert(buf.size() >= INET6_ADDRSTRLEN);
		}

		char const *res = inet_ntop(AF_INET6, &sa->sin6_addr, buf.data(), buf.size());
		if (NULL == res)
			return str_ref();

		char *p = (char*)res;
		p += std::strlen(p);

		if (print_port)
		{
			p += snprintf(p, buf.end() - p, ".%hu", ntohs(sa->sin6_port));
		}

		return str_ref(res, p);
	}

	inline str_ref os_sockaddr_print_to(buffer_ref buf, struct sockaddr const *sa, bool print_port = true)
	{
		switch (sa->sa_family) {
		default:
			// default = AF_INET
		case AF_INET:
			return os_sockaddr_print_to(buf, reinterpret_cast<struct sockaddr_in const*>(sa), print_port);
		case AF_INET6:
			return os_sockaddr_print_to(buf, reinterpret_cast<struct sockaddr_in6 const*>(sa), print_port);
		}
	}

	inline str_ref os_sockaddr_print_to(buffer_ref buf, struct sockaddr_storage const *sa, bool print_port = true)
	{
		return os_sockaddr_print_to(buf, (struct sockaddr const*)sa, print_port);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct_type, arg_type, arg_call, buf_size) 	\
	template<> 																		\
	struct type_tunnel<struct_type> 												\
	{																				\
		typedef meow::tmp_buffer<buf_size> tmp_buffer_t; 							\
		static str_ref call(arg_type sa, tmp_buffer_t const& buf = tmp_buffer_t())	\
		{																			\
			return os_sockaddr_print_to(buffer_ref(buf.begin(), buf.end()), arg_call sa, true); \
		}																			\
	};																				\
/**/

	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr const*, sockaddr const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr*,       sockaddr const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr,        sockaddr const&, &, INET6_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_storage const*, sockaddr_storage const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_storage*,       sockaddr_storage const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_storage,        sockaddr_storage const&, &, INET6_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in const*, sockaddr_in const*,  , INET_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in*,       sockaddr_in const*,  , INET_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in,        sockaddr_in const&, &, INET_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in6 const*, sockaddr_in6 const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in6*,       sockaddr_in6 const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL(struct sockaddr_in6,        sockaddr_in6 const&, &, INET6_ADDRSTRLEN + port_strlen);

#undef MEOW_DEFINE_SOCKADDR_TYPE_TUNNEL

#define MEOW_DEFINE_SOCKADDR_FUNCTIONS(arg_type, arg_call, buf_size) 	\
	inline str_ref sockaddr_tmp(										\
			  arg_type sa												\
			, meow::tmp_buffer<buf_size> const& buf = meow::tmp_buffer<buf_size>()) \
	{																	\
		return os_sockaddr_print_to(buffer_ref(buf.begin(), buf.end()), arg_call sa, true); \
	} 																	\
	inline str_ref sockaddr_as_ip(										\
			  arg_type sa												\
			, meow::tmp_buffer<buf_size> const& buf = meow::tmp_buffer<buf_size>()) \
	{																	\
		return os_sockaddr_print_to(buffer_ref(buf.begin(), buf.end()), arg_call sa, false); \
	} 																	\
/**/

	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr const&, &, INET6_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_storage const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_storage const&, &, INET6_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_in const*,  , INET_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_in const&, &, INET_ADDRSTRLEN + port_strlen);

	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_in6 const*,  , INET6_ADDRSTRLEN + port_strlen);
	MEOW_DEFINE_SOCKADDR_FUNCTIONS(struct sockaddr_in6 const&, &, INET6_ADDRSTRLEN + port_strlen);

#undef MEOW_DEFINE_SOCKADDR_FUNCTIONS

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__SOCKADDR_HPP_

