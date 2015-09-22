#include <iostream>
#include <string>

#include <cpl/flags.hpp>
#include <cpl/sockaddr.hpp>
#include <cpl/tcp_socket.hpp>
#include <cpl/tcp_connection.hpp>

#include "flags.hpp"
#include "node.hpp"
#include "peer.hpp"

const char* NAME    = "failure-detector";
const char* VERSION = "0.0.1";

void on_accept(std::vector<std::unique_ptr<Peer>>&, std::unique_ptr<cpl::net::TCP_Connection>);

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

	auto node = std::make_shared<Node>();
	int status = node->start(addr_str);
	if (status < 0) {
		std::cerr << "failed to start failure-detector" << std::endl;
		return 1;
	}

	std::cerr << "failure-detector listening on " << addr_str << std::endl;
	node->run();
	return 0;
}
