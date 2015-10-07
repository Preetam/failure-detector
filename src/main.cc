#include <iostream>
#include <string>

#include <cpl/flags.hpp>
#include <cpl/net/sockaddr.hpp>
#include <cpl/net/tcp_socket.hpp>
#include <cpl/net/tcp_connection.hpp>

#include "flags.hpp"
#include "node/node.hpp"

const char* NAME    = "failure-detector";
const char* VERSION = "0.0.1";

int
main(int argc, char* argv[]) {
	// Local listen address
	std::string addr_str;
	// Peers to initially connect to
	std::vector<cpl::net::SockAddr> peer_addrs;

	// Flags
	cpl::Flags flags(NAME, VERSION);
	flags.add_option("--help", "-h", "show help documentation", show_help, &flags);
	flags.add_option("--listen", "-l", "set listen address", set_listen_string, &addr_str);
	flags.add_option("--peers", "-p", "list of peers", add_peers, &peer_addrs);
	flags.parse(argc, argv);

	// Check required flags
	if (addr_str == "") {
		std::cerr << "--listen flag unset. Please specify a listen address." << std::endl;
		return 1;
	}

	auto node = std::make_shared<Node>();
	// Connect to each specified peer
	for (int i = 0; i < peer_addrs.size(); i++) {
		node->connect_to_peer(peer_addrs[i]);
	}
	int status = node->start(addr_str);
	if (status < 0) {
		std::cerr << "failed to start failure-detector" << std::endl;
		return 1;
	}

	std::cerr << "failure-detector listening on " << addr_str << std::endl;
	node->run();
	// Unreachable
	return 0;
}
