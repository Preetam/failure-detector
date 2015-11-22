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

#include "message/decode.hpp"
#include "message/message_times.hpp"
#include "message_queue/message_queue.hpp"
#include "message/identity_message.hpp"

class Peer
{
	using uq_tcp_conn = std::unique_ptr<cpl::net::TCP_Connection>;

public:
	Peer(int index,
		 uq_tcp_conn conn,
		 std::shared_ptr<Message_Queue> mq,
		 std::shared_ptr<cpl::Semaphore> close_notify_sem)
	: m_index(index),
	  m_id(0),
	  m_address(""),
	  m_conn(std::move(conn)),
	  m_mq(mq),
	  m_send_mq(std::make_shared<Message_Queue>(16)),
	  m_run_thread(true),
	  m_mtx(std::make_shared<cpl::RWMutex>()),
	  m_done(false)
	{
		LOG(INFO) << index_prefix() << "new peer created";
		m_thread = std::make_unique<std::thread>([this, close_notify_sem]() {
			run(close_notify_sem);
		});
	}

	Peer& operator =(Peer&& rhs)
	{
		rhs.m_run_thread = false;
		m_address = rhs.m_address;
		m_conn = std::move(rhs.m_conn);
		rhs.m_conn = nullptr;
		return *this;
	}

	// Disable copying.
	Peer(Peer& rhs) = delete;
	Peer& operator =(Peer& rhs) = delete;

	uint64_t
	id() const
	{
		return m_id;
	}

	void
	set_id(uint64_t id)
	{
		cpl::RWLock lk(*m_mtx, cpl::RWLock::Writer);
		m_id = id;
	}

	void
	set_address(const std::string& address)
	{
		cpl::RWLock lk(*m_mtx, cpl::RWLock::Writer);
		LOG(INFO) << index_prefix() << "has address '" << address << "'";
		m_address = address;
	}

	void
	set_identity_message(const IdentityMessage& m)
	{
		m_ident_msg = m;
	}

	void
	send_message(std::unique_ptr<Message> m)
	{
		m_send_mq->push(std::move(m));
	}

	void
	stop()
	{
		m_run_thread = false;
	}

	bool
	done()
	{
		return m_done;
	}

	~Peer()
	{
		m_run_thread = false;
		m_thread->join();
		LOG(INFO) << index_prefix() << "peer deleted";
	}

private:
	// Local peer index
	int                             m_index;
	// Unique peer ID (sent by the peer or prespecified)
	uint64_t                        m_id;
	// Reconnection address (sent by peer)
	std::string                     m_address;
	uq_tcp_conn                     m_conn;
	std::shared_ptr<Message_Queue>  m_mq;
	std::shared_ptr<Message_Queue>  m_send_mq;
	std::atomic<bool>               m_run_thread;
	std::unique_ptr<std::thread>    m_thread;
	std::shared_ptr<cpl::RWMutex>   m_mtx;
	std::atomic<bool>               m_done;
	IdentityMessage                 m_ident_msg;

	void
	run(std::shared_ptr<cpl::Semaphore>);

	void
	reconnect();

	int
	read_message();

	int
	write_message();

	inline std::string
	index_prefix()
	{
		return "[" + std::to_string(m_index) + "] ";
	}
}; // Peer
