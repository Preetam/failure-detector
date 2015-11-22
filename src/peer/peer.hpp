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

#include "message/message_times.hpp"
#include "message_queue/message_queue.hpp"

class Peer
{
public:
	Peer(int index,
		 std::unique_ptr<cpl::net::TCP_Connection> conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: m_index(index),
	  m_id(0),
	  m_address(""),
	  m_run_thread(true)
	{ 
		LOG(INFO) << "new peer created with index " << index;
	}

private:
	// Local peer index
	int m_index;
	// Unique peer ID (sent by the peer or prespecified)
	uint64_t m_id;
	// Reconnection address (sent by peer)
	std::string m_address;

	std::atomic<bool> m_run_thread;
	std::unique_ptr<std::thread> m_thread;

	void
	run();
}; // Peer
