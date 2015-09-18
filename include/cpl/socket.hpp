#pragma once

#include <unistd.h>     // close
#include <sys/socket.h> // socket

#include <string> // std::string

#include "ip.hpp"
#include "sockaddr.hpp"

namespace cpl
{
namespace net
{

class Socket
{
public:
	Socket()
	{
	}

	~Socket()
	{
		close(fd);
	}

	virtual int bind(std::string address, int port) throw() = 0;

protected:
	int fd;
	SockAddr local_address;

private:
	Socket(const Socket&);
	Socket& operator = (const Socket&);
}; // Socket

} // net
} // cpl
