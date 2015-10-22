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
	Peer(int local_id,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: local_id(local_id), unique_id(0), address(""),
	  conn(std::move(conn)),
	  mq(mq), active(true), valid(false), run_listener(true),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now())
	{
		LOG("new peer connected with local_id " << local_id);
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
	stop()
	{
		run_listener = false;
		thread->join();
		thread = nullptr;
	}

	std::unique_ptr<cpl::net::TCP_Connection>
	get_conn()
	{
		valid = false;
		active = false;
		return std::move(conn);
	}

	void
	update_conn(std::unique_ptr<cpl::net::TCP_Connection> new_connection)
	{
		conn = std::move(new_connection);
		last_update = std::chrono::steady_clock::now();
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
		active = true;
	}

	void
	reconnect();

	~Peer()
	{
		LOG("peer disconnected");
		thread->join();
	}

public:
	int local_id;
	uint64_t unique_id;
	std::string address;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> active;
	std::atomic<bool> valid;
	std::atomic<bool> run_listener;
	std::chrono::time_point<std::chrono::steady_clock> last_update;

	void
	read_messages();
}; // Peer
