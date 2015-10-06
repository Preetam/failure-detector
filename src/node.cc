#include "node.hpp"

int
Node :: start(std::string address) {
	cpl::net::SockAddr addr;
	int status;
	status = addr.parse(address);
	if (status < 0) {
		return status;
	}
	status = sock.bind(addr);
	if (status < 0) {
		return status;
	}
	status = sock.listen();
	if (status < 0) {
		return status;
	}
	return 0;
}

void
Node :: run() {
	std::thread cleanup([this]() {
		cleanup_nodes();
	});
	while (true) {
		auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
		int status = 0;
		if ( (status = sock.accept(conn_ptr.get())) == 0) {
			conn_ptr->set_timeout(1,0);
			on_accept(std::move(conn_ptr));
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			} else {
				break;
			}
		}
	}
}

void
Node :: cleanup_nodes() {
	while (true) {
		close_notify_sem->acquire();
		peers_lock->lock();
		for (int i = 0; i < peers->size(); i++) {
			if (!(*peers)[i]->is_active()) {
				peers->erase(peers->begin()+i);
				i--;
			}
		}
		peers_lock->unlock();
	}
}

void
Node :: on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
	auto peer = std::make_unique<Peer>(std::move(conn_ptr), mq, close_notify_sem);
	peers_lock->lock();
	peers->push_back(std::move(peer));
	peers_lock->unlock();
}
