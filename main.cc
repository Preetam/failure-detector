// cpl::net::IP
#include <cpl/include/ip.hpp>

// cpl::net::UDP_Socket
#include <cpl/include/udp_socket.hpp>

// cpl::net::Sockaddr
#include <cpl/include/sockaddr.hpp>

// std::cout
#include <iostream>

// std::thread
#include <thread>

// std::chrono
#include <chrono>

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

	try {
		sock.bind(listen_ip.string(), listen_port);
	} catch(...) {
		std::cout << "unable to listen on " << listen_ip << ":" << listen_port << "!" << std::endl;
		exit(1);
	}

	std::cout << "failure detector listening on " << listen_ip << ":" << listen_port << std::endl;

	std::thread listener([&sock]() {
		uint8_t buf[16000];
		while (true) {
			cpl::net::SockAddr saddr;
			int len = sock.recvfrom(buf, 16000, 0, &saddr);
			std::cout << "Message from " <<
				saddr.address() << ":" << saddr.port() << ": " <<
				std::string(reinterpret_cast<const char*>(buf), (size_t)len) << std::endl;
		}
	});

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
