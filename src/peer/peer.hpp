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

	Peer(int local_id,
		 std::string address,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: local_id(local_id), unique_id(0), address(address),
	  mq(mq), active(true), valid(false), run_listener(true),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now())
	{
		LOG("new peer connected with local_id " << local_id);
	}

	// send sends a Message to the Peer.
	void
	send(std::unique_ptr<Message>);

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
		valid = false;
		active = false;
		thread = nullptr;
	}

	std::unique_ptr<cpl::net::TCP_Connection>
	get_conn()
	{
		run_listener = false;
		valid = false;
		active = false;
		auto temp_conn = std::move(conn);
		conn = nullptr;
		return std::move(temp_conn);
	}

	void
	update_conn(std::unique_ptr<cpl::net::TCP_Connection> new_connection)
	{
		run_listener = false;
		conn = std::move(new_connection);
		last_update = std::chrono::steady_clock::now();
		run_listener = true;
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
		if (thread) {
			thread->join();
		}
	}

public:
	int local_id;
	uint64_t unique_id;
	std::string address;

	// Is this peer responding to pings?
	// Is it sending valid data?
	std::atomic<bool> active;

	// Can we reconnect to this peer?
	// Do we have an identity?
	std::atomic<bool> valid;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> run_listener;
	std::chrono::time_point<std::chrono::steady_clock> last_update;

	void
	read_messages();
}; // Peer
