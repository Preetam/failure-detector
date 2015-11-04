#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include <glog/logging.h>
#include <cpl/net/tcp_connection.hpp>
#include <cpl/semaphore.hpp>
#include <cpl/rwmutex.hpp>

#include "../message_queue/message_queue.hpp"

class Peer
{
public:
	Peer(int local_id,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: local_id(local_id),
	  unique_id(0),
	  address(""),
	  conn(std::move(conn)),
	  mq(mq),
	  active(true),
	  valid(false),
	  run_listener(true),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now()),
	  has_valid_connection(true)
	{ 
		LOG(INFO) << "new peer connected with local_id " << local_id;
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	Peer(int local_id,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::string address,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: local_id(local_id),
	  unique_id(0),
	  address(address),
	  conn(std::move(conn)),
	  mq(mq),
	  active(false),
	  valid(true), // since we have a reconnection address
	  run_listener(true),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now()),
	  has_valid_connection(true)
	{
		LOG(INFO) << "new peer connected with local_id " << local_id;
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	Peer(int local_id,
		 std::string address,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: local_id(local_id), unique_id(0), address(address),
	  mq(mq), active(false), valid(true), run_listener(true),
	  close_notify_sem(close_notify_sem),
	  last_update(std::chrono::steady_clock::now()),
	  has_valid_connection(false)
	{
		LOG(INFO) << "new peer connected with local_id " << local_id;
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	// send sends a Message to the Peer.
	void
	send(std::unique_ptr<Message>);

	int
	ms_since_last_active()
	{
		std::lock_guard<cpl::Mutex> lk(update_lock);
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now-last_update).count();
	}

	int
	ms_since_last_reconnect()
	{
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now-last_reconnect).count();
	}

	std::unique_ptr<cpl::net::TCP_Connection>
	get_conn()
	{
		cpl::RWLock lk(connection_lock, false);
		has_valid_connection = false;
		valid = false;
		active = false;
		auto temp_conn = std::move(conn);
		conn = nullptr;
		return std::move(temp_conn);
	}

	void
	use_conn(std::unique_ptr<cpl::net::TCP_Connection> new_connection)
	{
		cpl::RWLock lk(connection_lock, false);
		conn = std::move(new_connection);
		has_valid_connection = true;
		last_update = std::chrono::steady_clock::now();
		active = true;
		valid = true;
	}

	void
	mark_updated()
	{
		std::lock_guard<cpl::Mutex> lk(update_lock);
		last_update = std::chrono::steady_clock::now();
	}

	void
	reconnect();

	~Peer()
	{
		run_listener = false;
		LOG(INFO) << "peer[" << local_id << "] disconnected";
		thread->join();
	}

public:
	// Local peer index
	int local_id;
	// Unique peer ID (sent by the peer or prespecified)
	uint64_t unique_id;
	// Reconnection address (send by peer)
	std::string address;

	// Is this peer responding to pings?
	// Is it sending valid data?
	std::atomic<bool> active;

	// Can we reconnect to this peer?
	std::atomic<bool> valid;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> run_listener;
	std::chrono::time_point<std::chrono::steady_clock> last_reconnect;

	cpl::RWMutex connection_lock;
	bool has_valid_connection;

	cpl::Mutex update_lock;
	std::chrono::time_point<std::chrono::steady_clock> last_update;

	void
	read_messages();
}; // Peer
