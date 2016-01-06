#pragma once

#include <map>
#include <unordered_map>
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

private:
	std::unordered_map<int, shared_peer> m_peers;
	std::map<uint64_t, int>              m_id_to_index;
	cpl::Mutex                           m_mtx;
}; // PeerRegistry
