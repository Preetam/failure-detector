#pragma once

#include <uv.h>
#include <cstdint>

class Peer
{
public:
	Peer(std::unique_ptr<uv_tcp_t> conn)
	: m_active(true)
	, m_tcp(std::move(conn))
	{
		m_tcp->data = this;
		m_timer = std::make_unique<uv_timer_t>();
		uv_timer_init(m_tcp->loop, m_timer.get());
		m_timer->data = this;
	}

	void run()
	{
		uv_read_start((uv_stream_t*)m_tcp.get(),
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
				buf->base = new char[suggested_size];
				buf->len = suggested_size;
			},
			[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
				if (nread < 0) {
					// Got an error.
					uv_close((uv_handle_t*)stream, [](uv_handle_t* stream) {
						LOG(INFO) << "connection closed";
						((Peer*)(stream->data))->m_tcp = nullptr;
					});
					return;
				}
				LOG(INFO) << "got " << nread << " bytes of data";
				LOG(INFO) << std::string(buf->base);
				if (buf->base != nullptr) {
					delete buf->base;
				}
			}
		);
		uv_timer_start(m_timer.get(), [](uv_timer_t* timer) {
			LOG(INFO) << "periodic run for Peer";
		}, 1000, 1000);
	}

	void stop()
	{
		// Stop the periodic timer.
		uv_timer_stop(m_timer.get());
		m_active = false;
	}

	~Peer()
	{
		if (m_tcp != nullptr) {
			uv_close((uv_handle_t*)(m_tcp.get()), [](uv_handle_t* stream) {
				((Peer*)(stream->data))->m_tcp = nullptr;
			});
		}
	}

private:
	bool                        m_active;
	bool                        m_valid;
	std::unique_ptr<uv_tcp_t>   m_tcp;
	std::unique_ptr<uv_timer_t> m_timer;
}; // Peer
