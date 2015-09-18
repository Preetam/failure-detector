#pragma once

#include <memory>

#include "socket.hpp"
#include "sockaddr.hpp"
#include "tcp_connection.hpp"

namespace cpl
{
namespace net
{

class TCP_Socket : public Socket
{
public:
	inline int
	listen()
	{
		return listen(128);
	}

	inline int
	listen(int backlog)
	{
		if (::listen(fd, backlog) < 0) {
			return -1;
		}
		return 0;
	}

	inline int
	bind(SockAddr& address)
	{
		fd = socket(address.ip.family, SOCK_STREAM, 0);
		if (fd < 0) {
			return -1;
		}
		struct sockaddr_storage addr;
		socklen_t size;
		if (address.ip.family == AF_INET) {
			auto addr_in = reinterpret_cast<struct sockaddr_in*>(&addr);
			addr_in->sin_family = address.ip.family;
			addr_in->sin_addr = address.ip.addr.v4;
			addr_in->sin_port = htons(address.port);
			size = sizeof(sockaddr_in);
		} else {
			auto addr_in6 = reinterpret_cast<struct sockaddr_in6*>(&addr);
			addr_in6->sin6_family = address.ip.family;
			addr_in6->sin6_addr = address.ip.addr.v6;
			addr_in6->sin6_port = htons(address.port);
			size = sizeof(sockaddr_in6);
		}
		if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), size) < 0) {
			return -2;
		}
		return 0;
	}

	inline int
	bind(std::string ip_str, int port) throw()
	{
		IP ip;
		if (ip.set(ip_str) < 0) {
			return -1;
		}
		local_address = SockAddr(ip, port);
		return bind(local_address);
	}

	inline int
	accept(TCP_Connection* conn) throw()
	{
		struct sockaddr_storage remote;
		socklen_t remote_addr_size = sizeof(remote);
		int newfd = ::accept(fd, reinterpret_cast<struct sockaddr*>(&remote), &remote_addr_size);
		if (newfd < 0) {
			return -1;
		}
		SockAddr remote_address(remote);
		*conn = std::move(TCP_Connection(newfd, local_address, remote_address));
		return 0;
	}
}; // TCP_Socket

} // net
} // cpl
