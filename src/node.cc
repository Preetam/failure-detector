#include "node.hpp"

int
Node :: start(std::string address) {
	peers = std::make_shared<std::unordered_map<int,std::shared_ptr<Peer>>>();
	id_counter = 0;
	cpl::net::SockAddr addr;
	int status;
	status = addr.parse(address);
	if (status < 0) {
		return status;
	}

	status = _sock.bind(addr);
	if (status < 0) {
		return status;
	}
	status = _sock.listen();
	if (status < 0) {
		return status;
	}
	return 0;
}

void
Node :: run() {
	while (true) {
		auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
		if (_sock.accept(conn_ptr.get()) == 0) {
			on_accept(std::move(conn_ptr));
			id_counter++;
		} else {
		}
	}
}

void
Node :: on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
	auto peer = std::make_shared<Peer>(id_counter, std::move(conn_ptr),
		std::packaged_task<void(int)>([this](int id) {
		peers->erase(id);
	}));
	threads.push_back(std::make_unique<std::thread>([peer]() {
		peer->run();
	}));
	peers->insert({id_counter, peer});
}
