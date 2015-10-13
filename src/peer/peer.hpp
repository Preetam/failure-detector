#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <iostream>

#include <cpl/net/tcp_connection.hpp>
#include <cpl/semaphore.hpp>

#include "../message_queue/message_queue.hpp"

class Peer
{
public:
	Peer(int id,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: id(id),
	  conn(std::move(conn)),
	  mq(mq), active(true),
	  close_notify_sem(close_notify_sem)
	{
		std::cerr << "new peer connected" << std::endl;
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	// send sends a Message to the Peer.
	void
	send(Message);

	// is_active returns true if the Peer is still active.
	bool
	is_active()
	{
		return active;
	}

	~Peer()
	{
		std::cerr << "peer disconnected" << std::endl;
		thread->join();
	}

public:
	int id;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> active;

	void
	read_messages();
}; // Peer
