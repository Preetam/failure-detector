#pragma once

#include <memory>

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
		m_peers[index] = peer;
	}

	void
	cleanup_done_peers()
	{
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

}; // PeerRegistry
