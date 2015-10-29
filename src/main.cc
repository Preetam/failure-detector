#include <string>
#include <signal.h>
#include <iostream>

#include <glog/logging.h>
#include <cpl/flags.hpp>
#include <cpl/net/sockaddr.hpp>
#include <cpl/net/tcp_socket.hpp>
#include <cpl/net/tcp_connection.hpp>

#include "flags.hpp"
#include "node/node.hpp"

const std::string NAME    = "failure-detector";
const std::string VERSION = "0.0.2";

int
main(int argc, char* argv[]) {
	// Logging setup.
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = 1;
	LOG(INFO) << "Set up logging";

	// Ignore SIGPIPE. This way we don't exit if we attempt to
	// write to a closed connection.
	signal(SIGPIPE, SIG_IGN);

	// Local listen address
	std::string addr_str;
	// Peers to initially connect to.
	// These are automatically marked as valid.
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
	// Connect to each specified peer.
	// If a connection can't be established, the peer is
	// marked as inactive and reconnections will be attempted.
	for (int i = 0; i < peer_addrs.size(); i++) {
		node->connect_to_peer(peer_addrs[i]);
	}

	LOG(INFO) << NAME << " " << VERSION << " listening on " << addr_str;
	LOG(INFO) << "Unique ID is " << id;
	node->run();

	// Unreachable
	return 0;
}
