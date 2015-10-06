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
	Peer(std::unique_ptr<cpl::net::TCP_Connection> conn,
		std::shared_ptr<Message_Queue> mq,
		std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: conn(std::move(conn)),
	  mq(mq), active(true),
	  close_notify_sem(close_notify_sem)
	{
		std::cerr << "new peer connected" << std::endl;
		thread = std::make_unique<std::thread>([this]() {
			read_messages();
		});
	}

	bool
	is_active()
	{
		return active;
	}

	~Peer()
	{
		thread->join();
	}

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
	std::atomic<bool> active;

	void
	read_messages() {
		while (true) {
			uint8_t buf[16000];
			Message m;
			int len = conn->recv(buf, 16000, 0);
			if (len <= 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					continue;
				}
				break;
			}
			int status = m.unpack(buf, len);
			if (status != 0) {
				std::cerr << "message unpack status: " << status << std::endl;
				break;
			}
			mq->push(m);
		}
		std::cerr << "read_messages() ending" << std::endl;
		active = false;
		close_notify_sem->release();
	}
}; // Peer
