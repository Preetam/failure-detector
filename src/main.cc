#include <string>
#include <signal.h>
#include <iostream>

#include <glog/logging.h>
#include <cpl/flags.hpp>
#include <cpl/net/sockaddr.hpp>

#include "flags.hpp"
#include "node/node.hpp"

const std::string NAME    = "failure-detector";
const std::string VERSION = "0.0.4";

void new_conn_cb(uv_stream_t* server, int status) {
	if (status < 0) {
		LOG(ERROR) << "couldn't get a new connection";
	} else {
		LOG(INFO) << "got a new connection. accepting...";
		uv_tcp_t client;
		uv_tcp_init(server->loop, &client);
		if (uv_accept(server, (uv_stream_t*)&client) < 0) {
			LOG(ERROR) << "couldn't accept connection";
		}
	}
}

int
main(int argc, char* argv[]) {
	// Logging setup.
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = 1;

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
		std::cerr << "--id flag unset. Please specify a nonzero unique node ID." << std::endl;
		return 1;
	}

	Node n(id);
	if (n.start(addr_str) < 0) {
		LOG(WARNING) << "failed to start";
		return 1;
	}
	LOG(INFO) << "listening on " << addr_str << " with ID " << id;

	for (int i = 0; i < peer_addrs.size(); i++) {
		n.connect_to_peer(peer_addrs[i]);
	}

	n.run();

	// Unreachable
	return 1;
}
