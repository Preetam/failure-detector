#include "node.hpp"

int
Node :: start(std::string address) {
	cpl::net::SockAddr addr;
	int status;
	status = addr.parse(address);
	if (status < 0) {
		return status;
	}
	status = _sock.bind(addr);
	if (status < 0) {
		return status;
	}
	status = _sock.listen();
	if (status < 0) {
		return status;
	}
	return 0;
}

void
Node :: run() {
	while (true) {
		auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
		if (_sock.accept(conn_ptr.get()) == 0) {
			on_accept(std::move(conn_ptr));
		} else {
		}
	}
}

void
Node :: on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {

}
