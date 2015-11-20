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
	: index(index),
	  id(0),
	  address("")
	{ 
		LOG(INFO) << "new peer created with index " << index;
	}

public:
	// Local peer index
	int index;
	// Unique peer ID (sent by the peer or prespecified)
	uint64_t id;
	// Reconnection address (sent by peer)
	std::string address;
}; // Peer
