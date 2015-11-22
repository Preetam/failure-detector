#pragma once

#include <map>
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
	broadcast_message(Message& m)
	{
		for (auto& i : m_peers) {
			i.second->send_message(m.clone());
		}
	}

	uint64_t
	trusted_after(uint64_t id)
	{
		for (auto& i : m_id_to_index) {
			if (i.first > id) {
				return i.first;
			}
		}
		return 0;
	}

	void
	set_identity(int index, uint64_t id, std::string& address)
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		auto i = m_id_to_index.find(id);
		if (i == m_id_to_index.end()) {
			m_id_to_index[id] = index;
			m_peers[index]->set_id(id);
			m_peers[index]->set_address(address);
			return;
		}

		// Entry already exists.
		if (m_id_to_index[id] == index) {
			// Nothing to do.
			return;
		}

		LOG(INFO) << "already have ID " << id << " registered";
		uint64_t existing = m_id_to_index[id];
		auto existing_peer = m_peers[existing];
		auto new_peer = m_peers[index];
		*existing_peer = std::move(*new_peer);
		new_peer = nullptr;
		existing_peer->set_id(id);
		existing_peer->set_address(address);
		cleanup();
	}

	void
	cleanup_done_peers()
	{
		std::lock_guard<cpl::Mutex> lk(m_mtx);
		cleanup();
	}

private:
	std::unordered_map<int, shared_peer> m_peers;
	std::map<uint64_t, int>              m_id_to_index;
	cpl::Mutex                           m_mtx;

	void
	cleanup()
	{
		for (auto i = std::begin(m_peers); i != std::end(m_peers); ) {
			if (i->second == nullptr) {
				i = m_peers.erase(i);
			} else if (i->second->done()) {
				m_id_to_index.erase(i->second->id());
				i = m_peers.erase(i);
			} else {
				++i;
			}
		}
	}
}; // PeerRegistry
