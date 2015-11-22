#include "peer.hpp"

void
Peer :: run(std::shared_ptr<cpl::Semaphore> close_notify_sem) {
	LOG(INFO) << index_prefix() << "started peer thread";
	std::chrono::steady_clock::time_point last_reconnect;
	while (m_run_thread) {
		auto start = std::chrono::steady_clock::now();
		bool valid_conn = true;
		{
			cpl::RWLock lk(*m_mtx, cpl::RWLock::Reader);
			// Check if we have a valid connection.
			if (m_conn == nullptr) {
				// We don't.
				valid_conn = false;
			}
		}

		if (!valid_conn) {
			auto now = std::chrono::steady_clock::now();
			if (now - last_reconnect > std::chrono::seconds(5)) {
				reconnect();
				last_reconnect = now;
			}
		}
		if (valid_conn) {
			int status = read_message();
			if (status < 0) {
				cpl::RWLock lk(*m_mtx, cpl::RWLock::Writer);
				m_conn = nullptr;
				valid_conn = false;
			}
		}
		if (valid_conn) {
			int status = write_message();
			if (status < 0) {
				cpl::RWLock lk(*m_mtx, cpl::RWLock::Writer);
				m_conn = nullptr;
				valid_conn = false;
			}
		}
		auto elapsed = std::chrono::steady_clock::now() - start;
		if (elapsed < std::chrono::milliseconds(33)) {
			// Make sure we don't loop too frequently.
			std::this_thread::sleep_for(std::chrono::milliseconds(33) - elapsed);
		}
	}
	LOG(INFO) << index_prefix() << "ending peer thread";
	close_notify_sem->release();
	m_done = true;
}

void
Peer :: reconnect() {
	cpl::RWLock lk(*m_mtx, cpl::RWLock::Writer);
	if (m_address == "") {
		// Not a valid address.
		if (m_conn == nullptr) {
			// We're sure that we don't have a valid connection
			// and we can't reconnect, so let's terminate this peer.
			m_run_thread = false;
		}
		return;
	}
	LOG(INFO) << index_prefix() << "attempting to reconnect to " <<
		m_address;
	cpl::net::SockAddr addr;
	int status = addr.parse(m_address);
	if (status < 0) {
		// Not a valid address, so we may as well
		// set it to "".
		m_address = "";
		return;
	}
	auto new_connection = std::make_unique<cpl::net::TCP_Connection>();
	status = new_connection->connect(addr);
	if (status < 0) {
		LOG(WARNING) << index_prefix() << "couldn't reconnect to " <<
			m_address;
		return;
	}
	LOG(INFO) << index_prefix() << "reconnected to " <<
		m_address;
	new_connection->set_timeout(1,0);
	m_conn = std::move(new_connection);
	send_message(std::make_unique<IdentityMessage>(m_ident_msg.id, m_ident_msg.address));
}

int
Peer :: read_message() {
	uint8_t buf[16000];
	int len = 0;
	std::unique_ptr<Message> m;
	{
		cpl::RWLock lk(*m_mtx, cpl::RWLock::Reader);
		if (m_conn == nullptr) {
			return -1;
		}
		len = m_conn->recv(buf, 16000, 0);
	}
	if (len <= 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		return -1;
	}
	int status = decode_message(m, buf, len);
	if (status < 0) {
		return -2;
	}
	m->source = m_index;
	m_mq->push(std::move(m));
	return 0;
}

int
Peer :: write_message() {
	cpl::RWLock lk(*m_mtx, cpl::RWLock::Reader);
	if (m_conn == nullptr) {
		return -1;
	}
	while (m_send_mq->size() > 0) {
		uint8_t buf[16000];
		int packed_size = (m_send_mq->pop())->pack(buf, 16000);
		int status = m_conn->send(buf, packed_size, 0);
		if (status < 0) {
			return -2;
		}
	}
	return 0;
}
