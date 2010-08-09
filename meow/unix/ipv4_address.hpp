////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
// (c) 2005 based on code by Maxim Egorushkin
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__IPV4_ADDRESS_HPP_
#define MEOW_UNIX__IPV4_ADDRESS_HPP_

#include <netinet/in.h>
#include <stdint.h>

#include <meow/config/endian.hpp>
#include <meow/api_call_error.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace ipv4 {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct address_t
	{
		uint32_t addr;
		uint16_t port;

		address_t() : addr(0), port(0) {}
		address_t(uint32_t a, uint16_t p) : addr(a), port(p) {}
		address_t(sockaddr_in const& sa) : addr(ntohl(sa.sin_addr.s_addr)), port(ntohs(sa.sin_port)) {}

		sockaddr_in sockaddr() const
		{
			sockaddr_in sa = {};
			return fill_sockaddr(sa);
		}

		sockaddr_in* sockaddr_tmp(sockaddr_in const& a = sockaddr_in()) const
		{
			sockaddr_in *result = (sockaddr_in*)&a;
			fill_sockaddr(*result);
			return result;
		}

		socklen_t addrlen() const
		{
			return sizeof(sockaddr_in);
		}

		typedef sockaddr_in(address_t::*unspecified_bool_type)()const;
		operator unspecified_bool_type() const { return port ? &address_t::sockaddr : 0; }
		bool operator!() const { return 0 == port; }

	private:

		sockaddr_in& fill_sockaddr(sockaddr_in& sa) const
		{
			sa.sin_family = AF_INET;
			sa.sin_addr.s_addr = htonl(addr);
			sa.sin_port = htons(port);
			return sa;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	inline bool operator<(address_t const& a, address_t const& b)
	{
		return a.addr == b.addr ? a.port < b.port : a.addr < b.addr;
	}

	inline bool operator==(address_t const& a, address_t const& b)
	{
		return a.port == b.port && a.addr == b.addr;
	}

	inline bool operator!=(address_t const& a, address_t const& b)
	{
		return a.port != b.port || a.addr != b.addr;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace ipv4 {
////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio> // snprintf
#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct ipv4_address_just_ip_t
	{
		ipv4::address_t const& addr;
	};

	inline ipv4_address_just_ip_t addr_as_ip(ipv4::address_t const& a)
	{
		ipv4_address_just_ip_t const r = { addr: a };
		return r;
	}

	template<>
	struct type_tunnel<ipv4::address_t>
	{
		enum { buffer_size = sizeof("xxx.xxx.xxx.xxx:xxxxx") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(ipv4::address_t const& a, buffer_t const& buf = buffer_t())
		{
			#if (CPPUTIL_ARCH_BYTE_ORDER == 1234)
				enum { _3, _2, _1, _0 };
			#else
				enum { _0, _1, _2, _3 };
			#endif

			uint8_t const* b = reinterpret_cast<uint8_t const*>(&a.addr);
			ssize_t n = ::snprintf(buf.get(), buf.size(), "%hhu.%hhu.%hhu.%hhu:%hu", b[_0], b[_1], b[_2], b[_3], a.port);
			return str_ref(buf.begin(), n);
		}
	};

	template<>
	struct type_tunnel<ipv4_address_just_ip_t>
	{
		enum { buffer_size = sizeof("xxx.xxx.xxx.xxx") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(ipv4_address_just_ip_t const& a, buffer_t const& buf = buffer_t())
		{
			#if (CPPUTIL_ARCH_BYTE_ORDER == 1234)
				enum { _3, _2, _1, _0 };
			#else
				enum { _0, _1, _2, _3 };
			#endif

			uint8_t const* b = reinterpret_cast<uint8_t const*>(&a.addr.addr);
			ssize_t n = ::snprintf(buf.get(), buf.size(), "%hhu.%hhu.%hhu.%hhu", b[_0], b[_1], b[_2], b[_3]);
			return str_ref(buf.begin(), n);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__IPV4_ADDRESS_HPP_

