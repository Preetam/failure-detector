#pragma once

#include <uv.h>
#include <cstdint>

#include "message_queue/message_queue.hpp"
#include "message/decode.hpp"

const int READ_BUFFER_SIZE = 16*1024;

class Peer
{
public:
	Peer(std::unique_ptr<uv_tcp_t> conn, std::shared_ptr<Message_Queue> mq)
	: m_active(true)
	, m_tcp(std::move(conn))
	, m_mq(mq)
	, m_valid(false)
	{
		m_tcp->data = this;
		m_loop = m_tcp->loop;
	}

	Peer(cpl::net::SockAddr& addr, std::unique_ptr<uv_tcp_t> conn,
		 std::shared_ptr<Message_Queue> mq)
	: m_active(true)
	, m_tcp(std::move(conn))
	, m_mq(mq)
	, m_valid(true)
	{
		m_tcp->data = this;
		m_loop = m_tcp->loop;
		auto req = new uv_connect_t;
		req->data = this;
		struct sockaddr_storage sockaddr;
		addr.get_sockaddr(reinterpret_cast<struct sockaddr*>(&sockaddr));
		uv_tcp_connect(req, m_tcp.get(), reinterpret_cast<struct sockaddr*>(&sockaddr),
			[](uv_connect_t* req, int status) {
				if (status < 0) {
					return;
				}
				((Peer*)req->data)->run();
				delete req;
			});
	}

	uint64_t
	id()
	{
		return m_id;
	}

	Peer& operator =(Peer& rhs) = delete; // Disable copying.

	Peer& operator =(Peer&& rhs)
	{
		LOG(INFO) << "!!! move";
		// Close the old TCP stream if it's active.
		if (m_active && m_tcp != nullptr) {
			auto old_stream = m_tcp.release();
			uv_close((uv_handle_t*)old_stream, [](uv_handle_t* handle) {
				delete handle;
			});
		}
		m_tcp = std::move(rhs.m_tcp);
		m_tcp->data = this;
		rhs.m_valid = false;
		rhs.m_active = false;
		return *this;
	}

	void
	set_identity(const uint64_t id, const std::string& address)
	{
		m_id = id;
		m_address = address;
		m_valid = true;
	}

	void
	run()
	{
		m_last_reconnect = uv_hrtime();
		uv_read_start((uv_stream_t*)m_tcp.get(),
			// Buffer allocation callback
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
				buf->base = ((Peer*)handle->data)->m_alloc_buf;
				buf->len = READ_BUFFER_SIZE;
			},

			// On read callback
			[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
				auto peer = (Peer*)stream->data;
				if (nread < 0) {
					// Got an error.
					if (uv_is_active((uv_handle_t*)stream) == 0) {
						// Inactive; no need to close.
						LOG(INFO) << "uv_tcp_t is inactive";
						return;
					}
					uv_close((uv_handle_t*)stream, [](uv_handle_t* handle) {
						auto self = (Peer*)handle->data;
						LOG(INFO) << "connection closed";
						self->m_active = false;
						if (self->m_valid) {
							self->m_tcp = nullptr;
						}
					});
					return;
				}
				LOG(INFO) << "got " << nread << " bytes of data";
				for (int i = 0; i < nread; i++) {
					peer->m_read_buf.push_back(buf->base[i]);
					peer->m_pending_msg_size++;
				}
				auto msg_length = decode_message_length(peer->m_read_buf.data(),
					peer->m_read_buf.size());
				if (msg_length > 0 && peer->m_pending_msg_size >= msg_length) {
					peer->process_message_data(peer->m_read_buf.data(), msg_length);
					// Trim the buffer.
					peer->m_read_buf.erase(peer->m_read_buf.begin(),
						peer->m_read_buf.begin()+msg_length);
					peer->m_pending_msg_size -= msg_length;
				}
			}
		);
	}

	void
	send(Message* msg)
	{
		int size = msg->packed_size();
		auto buf = new uint8_t[size];
		uv_buf_t a[] = {
			{.base = (char*)buf, .len = (size_t)size}
		};
		msg->pack(buf, size);
		auto req = new uv_write_t;
		req->data = buf;
		uv_write(req, (uv_stream_t*)m_tcp.get(), a, 1, [](uv_write_t* req, int) {
			auto buf = (uint8_t*)(req->data);
			delete buf;
			delete req;
		});
	}

	void
	set_index(int index)
	{
		m_index = index;
	}

	bool
	done()
	{
		return !m_active && !m_valid;
	}

	~Peer()
	{
		LOG(INFO) << "~Peer()";
	}

private:
	void
	process_message_data(uint8_t* data, int size)
	{
		// TODO
		LOG(INFO) << "processing message data of size " << size;

		std::unique_ptr<Message> m;
		if (decode_message(m, data, size) >= 0) {
			m->source = m_index;
			m_mq->push(std::move(m));
		}
	}

	void
	reconnect()
	{
		LOG(INFO) << "reconnecting to " << m_address;
		if (m_tcp == nullptr) {
			m_tcp = std::make_unique<uv_tcp_t>();
			if (uv_tcp_init(m_loop, m_tcp.get()) < 0) {
				LOG(WARNING) << "uv_tcp_init failed";
			}
			m_tcp->data = this;
		}
		auto req = new uv_connect_t;
		req->data = this;
		cpl::net::SockAddr addr;
		if (addr.parse(m_address) < 0) {
			LOG(WARNING) << "failed to parse address: " << m_address;
		}
		struct sockaddr_storage sockaddr;
		addr.get_sockaddr(reinterpret_cast<struct sockaddr*>(&sockaddr));
		uv_tcp_connect(req, m_tcp.get(), reinterpret_cast<struct sockaddr*>(&sockaddr),
			[](uv_connect_t* req, int status) {
				if (status < 0) {
					LOG(WARNING) << "reconnect failed";
					auto self = (Peer*)req->data;
					uv_close((uv_handle_t*)self->m_tcp.get(), [](uv_handle_t* handle) {
						auto self = (Peer*)handle->data;
						self->m_tcp = nullptr;
					});
					delete req;
					return;
				}
				((Peer*)req->data)->run();
				((Peer*)req->data)->m_active = true;
				delete req;
			});
		m_last_reconnect = uv_hrtime();
	}

private:
	bool                           m_active;
	bool                           m_valid;
	std::unique_ptr<uv_tcp_t>      m_tcp;
	uv_loop_t*                     m_loop;
	std::shared_ptr<Message_Queue> m_mq;
	int                            m_index;
	uint64_t                       m_id;
	std::string                    m_address;
	uint64_t                       m_last_reconnect;

	char                           m_alloc_buf[READ_BUFFER_SIZE];
	std::vector<uint8_t>           m_read_buf;
	int                            m_pending_msg_size;
}; // Peer
