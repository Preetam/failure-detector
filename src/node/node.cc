#include "node.hpp"

int
Node :: start(std::string address) {
	// Create a libuv event loop.
	m_uv_loop = std::make_unique<uv_loop_t>();
	if (uv_loop_init(m_uv_loop.get()) < 0) {
		LOG(ERROR) << "failed to initialize event loop";
		return 1;
	}

	// Parse address string.
	struct sockaddr_storage sockaddr;
	cpl::net::SockAddr addr;
	if (addr.parse(address) < 0) {
		LOG(ERROR) << "failed to parse listen address";
		return 1;
	}
	addr.get_sockaddr(reinterpret_cast<struct sockaddr*>(&sockaddr));

	// Create a TCP handle.
	m_tcp = std::make_unique<uv_tcp_t>();
	if (uv_tcp_init(m_uv_loop.get(), m_tcp.get()) < 0) {
		LOG(ERROR) << "failed to initialize TCP handle";
		return 1;
	}
	m_tcp->data = this;
	auto status = uv_tcp_bind(m_tcp.get(), reinterpret_cast<struct sockaddr*>(&sockaddr), 0);
	if (status < 0) {
		LOG(ERROR) << "failed to bind to address: " << uv_strerror(status);
		return 1;
	}

	if (uv_listen((uv_stream_t*)m_tcp.get(), 8, Node::on_connect) < 0) {
		LOG(ERROR) << "uv_listen failed";
		return 1;
	}

	m_listen_address = address;
	return 0;
}

void
Node :: run() {
	uv_timer_t periodic_timer;
	uv_timer_init(m_uv_loop.get(), &periodic_timer);
	periodic_timer.data = this;
	uv_timer_start(&periodic_timer, [](uv_timer_t* timer) {
		LOG(INFO) << "periodic run for node (id " << ((Node*)timer->data)->m_id << ")";
	},
	1000, 1000);
	if (uv_run(m_uv_loop.get(), UV_RUN_DEFAULT) < 0) {
		LOG(ERROR) << "failed to run event loop";
		return;
	}
	uv_loop_close(m_uv_loop.get());
}
