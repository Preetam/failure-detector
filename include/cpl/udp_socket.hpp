#pragma once

#include <cstring> // memset

#include "socket.hpp" 
#include "sockaddr.hpp"

namespace cpl
{
namespace net
{

class UDP_Socket : public Socket
{
public:
	inline int
	bind(std::string ip, int port) throw()
	{
		local_address = SockAddr(IP(ip), port);
		fd = socket(local_address.ip.family, SOCK_DGRAM, 0);
		if (fd < 0) {
			return -1;
		}
		struct sockaddr_storage addr;
		socklen_t size;
		if (local_address.ip.family == AF_INET) {
			auto addr_in = reinterpret_cast<struct sockaddr_in*>(&addr);
			addr_in->sin_family = local_address.ip.family;
			addr_in->sin_addr = local_address.ip.addr.v4;
			addr_in->sin_port = htons(port);
			size = sizeof(sockaddr_in);
		} else {
			auto addr_in6 = reinterpret_cast<struct sockaddr_in6*>(&addr);
			addr_in6->sin6_family = local_address.ip.family;
			addr_in6->sin6_addr = local_address.ip.addr.v6;
			addr_in6->sin6_port = htons(port);
			size = sizeof(sockaddr_in6);
		}
		if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), size) < 0) {
			return -2;
		}
		return 0;
	}

	inline int
	recvfrom(void* buf, size_t len, int flags, SockAddr* addr)
	{
		struct sockaddr_storage source;
		socklen_t source_len = sizeof source;
		memset(&source, 0, source_len);
		auto ret = (int)(::recvfrom(fd, buf, len, flags, (struct sockaddr*)&source, &source_len));
		if (ret < 0) {
			return ret;
		}
		addr->set(source);
		return ret;
	}

	inline int
	sendto(const void* buf, size_t len, int flags, IP ip, int port)
	{
		struct sockaddr_storage dest;
		socklen_t dest_len = sizeof dest;
		memset(&dest, 0, dest_len);
		if (ip.family == AF_INET) {
			auto dest_in = reinterpret_cast<struct sockaddr_in*>(&dest);
			dest_in->sin_family = AF_INET;
			dest_in->sin_port = htons((unsigned short)port);
			dest_in->sin_addr = ip.addr.v4;
		} else {
			auto dest_in6 = reinterpret_cast<struct sockaddr_in6*>(&dest);
			dest_in6->sin6_family = AF_INET6;
			dest_in6->sin6_port = htons((unsigned short)port);
			dest_in6->sin6_addr = ip.addr.v6;
		}
		auto ret = (int)(::sendto(fd, buf, len, flags,
			reinterpret_cast<struct sockaddr*>(&dest), dest_len));
		return ret;
	}

private:
	
}; // UDP_Socket

} // net
} // cpl
