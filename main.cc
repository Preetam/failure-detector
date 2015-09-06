// cpl::net::IP
#include <cpl/include/ip.hpp>

// cpl::net::TCP_Socket
#include <cpl/include/tcp_socket.hpp>
#include <cpl/include/tcp_connection.hpp>

// cpl::net::Sockaddr
#include <cpl/include/sockaddr.hpp>

// std::cout
#include <iostream>

// std::thread
#include <thread>

// std::chrono
#include <chrono>

#include <vector>
#include <memory>

#include "flags.hpp"
#include "peer.hpp"

int
main(int argc, char* argv[]) {
	cpl::net::IP listen_ip("127.0.0.1");
	int listen_port = 2020;
	std::vector<std::string> peers_vec;
	std::vector<Peer> peers;

	cpl::Flags flags("failure-detector", "0.0.1");
	flags.add_option("--help", "-h", "show help documentation", show_help, &flags);
	flags.add_option("--listen", "-l", "set listen address", set_listen, &listen_ip);
	flags.add_option("--port", "-p", "set listen port", set_listen_port, &listen_port);
	flags.add_option("--peers", "", "add peers", add_peers, &peers_vec);
	flags.parse(argc, argv);

	// Set up peers.

	for (auto p: peers_vec) {
		peers.push_back(Peer(p));
	}

	cpl::net::TCP_Socket sock;

	try {
		sock.bind(listen_ip.string(), listen_port);
		sock.listen();
	} catch (...) {
		std::cout << "unable to listen on " << listen_ip << ":" << listen_port << "!" << std::endl;
		exit(1);
	}

	std::cout << "failure detector listening on " << listen_ip << ":" << listen_port << std::endl;

	std::vector<std::unique_ptr<std::thread>> threads;

	while (true) {
		auto new_conn = std::make_unique<cpl::net::TCP_Connection>();

		sock.accept(new_conn.get());
		auto listener = std::make_unique<std::thread>(
			[](std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
				auto conn = conn_ptr.get();

				uint8_t buf[16000];
				while (true) {
					int len = conn->recv(buf, 16000, 0);
					if (len <= 0) {
						return;
					}
					std::cout << "Message from " <<
						conn->remote_address().ip << ":" <<
						conn->remote_address().port << ": " <<
						std::string(reinterpret_cast<const char*>(buf), (size_t)len) << std::endl;
				}
		}, std::move(new_conn));

		threads.push_back(std::move(listener));
	}

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
