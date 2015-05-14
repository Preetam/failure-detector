// cpl::net::IP
#include <cppl/include/ip.hpp>

// cpl::net::UDP_Socket
#include <cppl/include/udp_socket.hpp>

// cpl::net::Sockaddr
#include <cppl/include/sockaddr.hpp>

// std::cout
#include <iostream>

#include "flags.hpp"

int
main(int argc, char* argv[]) {
	cpl::net::IP listen_ip("127.0.0.1");
	int listen_port = 2020;

	cpl::Flags flags("failure-detector", "0.0.1");
	flags.add_option("--help", "-h", "show help documentation", show_help, &flags);
	flags.add_option("--listen", "-l", "set listen address", set_listen, &listen_ip);
	flags.add_option("--port", "-p", "set listen port", set_listen_port, &listen_port);
	flags.parse(argc, argv);

	cpl::net::UDP_Socket sock;
	sock.bind(listen_ip.string(), listen_port);

	std::cout << "failure detector listening on " << listen_ip << ":" << listen_port << std::endl;


	uint8_t buf[16000];
	while (true) {
		cpl::net::SockAddr saddr;
		int len = sock.recvfrom(buf, 16000, 0, &saddr);
		std::cout << "Message from " <<
			saddr.address() << ":" << saddr.port() << ": " <<
			std::string(reinterpret_cast<const char*>(buf), (size_t)len) << std::endl;
	}

	return 0;
}
