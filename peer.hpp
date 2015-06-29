#pragma once

#include <cpl/include/sockaddr.hpp>

class Peer
{
public:
	Peer(std::string address)
	: address(address)
	{
	}

	cpl::net::SockAddr address;

private:

}; // Peer
