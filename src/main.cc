#include <signal.h>
#include <iostream>
#include <string>

#include <cpl/flags.hpp>
#include <cpl/net/sockaddr.hpp>
#include <cpl/net/tcp_socket.hpp>
#include <cpl/net/tcp_connection.hpp>

#include "log.hpp"
#include "flags.hpp"
#include "node/node.hpp"

const char* NAME    = "failure-detector";
const char* VERSION = "0.0.1";

int
main(int argc, char* argv[]) {
	// Ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	// Local listen address
	std::string addr_str;
	// Peers to initially connect to
	std::vector<cpl::net::SockAddr> peer_addrs;
	uint64_t id = 0;

	// Flags
	cpl::Flags flags(NAME, VERSION);
	flags.add_option("--help", "-h", "show help documentation", show_help, &flags);
	flags.add_option("--listen", "-l", "set listen address", set_listen_string, &addr_str);
	flags.add_option("--peers", "-p", "list of peers", add_peers, &peer_addrs);
	flags.add_option("--id", "-i", "ID, unique among the cluster", set_id, &id);
	flags.parse(argc, argv);

	// Check required flags
	if (addr_str == "") {
		std::cerr << "--listen flag unset. Please specify a listen address." << std::endl;
		return 1;
	}

	if (id == 0) {
		std::cerr << "--id flag unset. Please specify a unique node ID." << std::endl;
		return 1;
	}

	auto node = std::make_shared<Node>(id);
	int status = node->start(addr_str);
	if (status < 0) {
		std::cerr << "failed to start " << NAME << std::endl;
		return 1;
	}
	// Connect to each specified peer
	for (int i = 0; i < peer_addrs.size(); i++) {
		node->connect_to_peer(peer_addrs[i]);
	}

	std::cerr << NAME << " " << VERSION << " listening on " << addr_str << std::endl;
	LOG("Unique ID is " << id);
	node->run();

	// Unreachable
	return 0;
}
