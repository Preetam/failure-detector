#pragma once

#include <memory>
#include <atomic>

#include "cpl/mutex.hpp"
#include "peer/peer.hpp"

class PeerRegistry
{
	using shared_peer = std::shared_ptr<Peer>;

public:
	PeerRegistry()
	{
	}

	void
	register_peer(int index, shared_peer peer)
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		m_peers[index] = peer;
	}

	void
	send_message_to_index(int index, std::unique_ptr<Message> m)
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		auto i = m_peers.find(index);
		if (i == m_peers.end()) {
			return;
		}
		i->second->send_message(std::move(m));
	}

	void
	send_message_to_id(uint64_t id, std::unique_ptr<Message> m)
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		for (auto& i : m_peers) {
			if (i.second->id() == id) {
				i.second->send_message(std::move(m));
			}
		}
	}

	void
	cleanup_done_peers()
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		for (auto i = std::begin(m_peers); i != std::end(m_peers); ) {
			if (i->second->done()) {
				i = m_peers.erase(i);
			} else {
				++i;
			}
		}
	}

private:
	std::unordered_map<int, shared_peer> m_peers;
	cpl::Mutex                           m_mtx;

}; // PeerRegistry
