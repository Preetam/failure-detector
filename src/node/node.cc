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
	
	m_last_leader_active = std::chrono::steady_clock::now();
	while (true) {
		auto start = std::chrono::steady_clock::now();
		// This is the main node loop.
		process_message();

		if (m_trusted_peer == 0 ||
			(std::chrono::steady_clock::now() - m_last_leader_active > std::chrono::seconds(1)) ) {
			m_trusted_peer = m_registry->trusted_after(m_trusted_peer);
			if (m_trusted_peer == 0 || m_trusted_peer > m_id) {
				// Become leader.
				LOG(INFO) << "became leader";
				m_trusted_peer = m_id;
			} else {
				LOG(INFO) << m_trusted_peer << " is now the leader";
			}
		}

		if (m_trusted_peer == m_id) {
			// Currently the leader.
			LeaderActiveMessage msg(m_id);
			m_registry->broadcast_message(msg);
			m_last_leader_active = std::chrono::steady_clock::now();
		}

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
			conn_ptr->set_timeout(0, 33);
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
	auto msg = std::make_unique<IdentityMessage>(m_id, m_listen_address);
	peer->set_identity_message(*msg);
	// Send our identity to the new peer.
	peer->send_message(std::move(msg));
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
		peer_conn->set_timeout(0,33);
	}
	auto next_id = ++m_index_counter;
	peer = std::make_shared<Peer>(next_id, std::move(peer_conn), m_mq, m_close_notify_sem);
	peer->set_address(address.str());
	auto msg = std::make_unique<IdentityMessage>(m_id, m_listen_address);
	peer->set_identity_message(*msg);
	// Send our identity to the new peer.
	peer->send_message(std::move(msg));
	m_registry->register_peer(next_id, peer);
}

void
Node :: process_message() {
	std::unique_ptr<Message> m;
	if (m_mq->pop_with_timeout(m, 50)) {
		//LOG(INFO) << "new message (type " << MSG_STR(m->type) << ")";
		switch (m->type) {
		case MSG_PING:
			handle_ping(*m);
			break;
		case MSG_PONG:
			handle_pong(*m);
			break;
		case MSG_IDENT:
			handle_ident(*m);
			break;
		case MSG_IDENT_REQUEST:
			handle_ident_request(*m);
			break;
		case MSG_LEADER_ACTIVE:
			handle_leader_active(*m);
		default:
			break;
		}
	}
}

void
Node :: handle_ping(const Message& m) {
	// TODO
}

void
Node :: handle_pong(const Message& m) {
	// TODO
}

void
Node :: handle_ident(const Message& m) {
	auto ident_msg = static_cast<const IdentityMessage&>(m);
	m_registry->set_identity(ident_msg.source, ident_msg.id, ident_msg.address);
	LOG(INFO) << ident_msg.source << " has ID " << ident_msg.id << " and address " << ident_msg.address;
}

void
Node :: handle_ident_request(const Message& m) {
	// TODO
}

void
Node :: handle_leader_active(const Message& m) {
	auto leader_active_msg = static_cast<const LeaderActiveMessage&>(m);
	if (leader_active_msg.id < m_trusted_peer) {
		LOG(INFO) << "received leader message from higher ranked leader (" <<
			leader_active_msg.id << "). " <<
			"Marking " << leader_active_msg.id << " as leader.";
		m_trusted_peer = leader_active_msg.id;
	}
	if (m_trusted_peer == leader_active_msg.id) {
		m_last_leader_active = std::chrono::steady_clock::now();
	}
}
