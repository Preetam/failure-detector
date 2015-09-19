#include <iostream>
#include <string>

#include <cpl/flags.hpp>
#include <cpl/sockaddr.hpp>
#include <cpl/tcp_socket.hpp>
#include <cpl/tcp_connection.hpp>

#include "flags.hpp"

const char* NAME    = "failure-detector";
const char* VERSION = "0.0.1";

void on_accept(std::unique_ptr<cpl::net::TCP_Connection>);

int
main(int argc, char* argv[]) {
	std::string addr_str;

	cpl::Flags flags(NAME, VERSION);
	flags.add_option("--help", "-h", "show help documentation", show_help, &flags);
	flags.add_option("--listen", "-l", "set listen address", set_listen_string, &addr_str);
	flags.parse(argc, argv);

	if (addr_str == "") {
		std::cerr << "--listen flag unset. Please specify a listen address." << std::endl;
		return 1;
	}

	cpl::net::SockAddr addr;
	if (addr.parse(addr_str) < 0) {
		std::cerr << "`" + addr_str + "`" << " is not a valid address." << std::endl;
		return 1;
	}

	cpl::net::TCP_Socket sock;
	int status;
	status = sock.bind(addr);
	if (status < 0) {
		std::cerr << "unable to bind to " << addr_str << " (status " << status << ")" << std::endl;
		exit(1);
	}
	status = sock.listen();
	if (status < 0) {
		std::cerr << "unable to listen on " << addr_str << std::endl;
		exit(1);
	}

	while (true) {
		auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
		if (sock.accept(conn_ptr.get()) == 0) {
			on_accept(std::move(conn_ptr));
		} else {
			std::cerr << "unable to accept connection" << std::endl;
			return -1;
		}
	}

	std::cerr << "failure-detector listening on " << addr_str << std::endl;
	return 0;
}

void
on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
	std::cerr << "accepted a new connection" << std::endl;
}
