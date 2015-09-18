#pragma once

#include <cpl/include/sockaddr.hpp>

class Peer
{
public:
	Peer(std::string address_string)
	{
		address.parse(address_string);
	}

	cpl::net::SockAddr address;

private:

}; // Peer
