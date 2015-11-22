#include "node.hpp"

int
Node :: start(std::string address) {
	cpl::net::SockAddr addr;
	int status;
	status = addr.parse(address);
	if (status < 0) {
		return status;
	}
	status = m_sock.bind(addr);
	if (status < 0) {
		return status;
	}
	status = m_sock.listen();
	if (status < 0) {
		return status;
	}
	m_listen_address = address;
	return 0;
}

void
Node :: run() {
	std::thread cleanup([this]() {
		cleanup_peers();
	});
	std::thread conn_accept_thread([this]() {
		handle_new_connections();
	});
	
	while (true) {
		auto start = std::chrono::steady_clock::now();
		// This is the main node loop.
		process_message();

		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start).count();
		if (dur > 1000) {
			LOG(WARNING) << "Main node loop iteration took over 1000 ms!";
		}
	}
	// Unreachable
}

void
Node :: cleanup_peers() {
	while (true) {
		// Wait until we're signaled that a connection
		// is closed.
		m_close_notify_sem->acquire();
		m_registry->cleanup_done_peers();
	}
}

void
Node :: handle_new_connections() {
	auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
	while (true) {
		int status = 0;
		if ( (status = m_sock.accept(conn_ptr.get())) == 0) {
			conn_ptr->set_timeout(1,0);
			on_accept(std::move(conn_ptr));
			conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
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
Node :: on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
	auto next_id = ++m_index_counter;
	auto peer = std::make_shared<Peer>(next_id, std::move(conn_ptr), m_mq, m_close_notify_sem);
	m_registry->register_peer(next_id, peer);
}

void
Node :: connect_to_peer(const cpl::net::SockAddr& address) {
	auto peer_conn = std::make_unique<cpl::net::TCP_Connection>();
	std::shared_ptr<Peer> peer;
	// Attempt to connect.
	int status = peer_conn->connect(address);
	if (status < 0) {
		LOG(WARNING) << "unable to connect to " << address;
		peer_conn = nullptr;
	} else {
		LOG(INFO) << "successfully connected to " << address;
		peer_conn->set_timeout(1,0);
	}
	auto next_id = ++m_index_counter;
	peer = std::make_shared<Peer>(next_id, std::move(peer_conn), m_mq, m_close_notify_sem);
	peer->set_address(address.str());
	m_registry->register_peer(next_id, peer);
}

void
Node :: process_message() {
	std::unique_ptr<Message> m;
	if (m_mq->pop_with_timeout(m, 50)) {
		LOG(INFO) << "new message (type " << MSG_STR(m->type) << ")";
		switch (m->type) {
		case MSG_PING:
			handle_ping(m.get());
			break;
		case MSG_PONG:
			handle_pong(m.get());
			break;
		case MSG_IDENT:
			handle_ident(m.get());
			break;
		case MSG_IDENT_REQUEST:
			handle_ident_request(m.get());
			break;
		default:
			break;
		}
	}
}

void
Node :: handle_ping(const Message* m) {
	// TODO
}

void
Node :: handle_pong(const Message* m) {
	// TODO
}

void
Node :: handle_ident(const Message* m) {
	// TODO
}

void
Node :: handle_ident_request(const Message* m) {
	// TODO
}
