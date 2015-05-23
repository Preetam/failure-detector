#pragma once

#include <cpl/include/address.hpp>

class Peer
{
public:
	Peer(std::string address)
	: address(address)
	{
	}

	cpl::net::Address address;

private:

}; // Peer
