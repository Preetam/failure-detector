#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include <cpl/net/tcp_connection.hpp>
#include <cpl/semaphore.hpp>

#include "../log.hpp"
#include "../message_queue/message_queue.hpp"

class Peer
{
public:
	Peer(int index,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: index(index), unique_id(0), address(""),
	  conn(std::move(conn)),
	  mq(mq), active(true), valid(false),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now())
	{
		LOG("new peer connected with index " << index);
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	// send sends a Message to the Peer.
	void
	send(std::unique_ptr<Message>);

	// is_active returns true if the Peer is still active.
	bool
	is_active()
	{
		return active;
	}

	// is_valid returns true if the Peer is valid.
	bool
	is_valid()
	{
		return valid;
	}

	int
	ms_since_last_active()
	{
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now-last_update).count();
	}

	void
	reconnect();

	~Peer()
	{
		LOG("peer disconnected");
		thread->join();
	}

public:
	uint64_t unique_id;
	std::string address;
	int index;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> active;
	std::atomic<bool> valid;
	std::chrono::time_point<std::chrono::steady_clock> last_update;

	void
	read_messages();
}; // Peer
