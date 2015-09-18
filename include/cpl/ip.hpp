#pragma once

#include <arpa/inet.h>

#include <string> // std::string
#include <iostream> // std::cout
#include <stdexcept> // std::invalid_argument

namespace cpl
{
namespace net
{

struct IP
{
	int family;
	union {
		in_addr v4;
		in6_addr v6;
	} addr;

	IP()
	: family(0)
	{
	}

	IP(const in_addr address)
	: family(AF_INET)
	{
		addr.v4 = address;
	}

	IP(const in6_addr address)
	: family(AF_INET6)
	{
		addr.v6 = address;
	}

	IP(const std::string& address)
	{
		set(address);
	}

	inline int
	set(const std::string& address)
	{
		auto s = inet_pton(AF_INET, address.c_str(), &addr.v4);
		if (s == 1) {
			// success
			family = AF_INET;
			return 0;
		}
		s = inet_pton(AF_INET6, address.c_str(), &addr.v6);
		if (s == 1) {
			// success
			family = AF_INET6;
			return 0;
		}
		return -1;
	}

	inline std::string
	string() const
	{
		if (family == AF_INET) {
			// v4
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, (void*)(&addr), str, INET_ADDRSTRLEN);
			return std::string(str);
		}
		if (family == AF_INET6) {
			// v6
			char str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, (void*)(&addr), str, INET6_ADDRSTRLEN);
			return std::string(str);
		}
		return "";
	}

	friend std::ostream& operator << (std::ostream& os, const IP& ip);
}; // IP

inline
std::ostream& operator << (std::ostream& os, const IP& ip) {
	os << ip.string();
	return os;
}

} // net
} // cpl
