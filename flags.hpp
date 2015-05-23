#pragma once

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>

#include <algorithm>

// cpl::Flags
#include <cpl/include/flags.hpp>

// cpl::net::IP
#include <cpl/include/ip.hpp>

void
show_help(std::string a, std::string b, void* d) {
	auto flags = reinterpret_cast<cpl::Flags*>(d);
	flags->print_usage();
	exit(0);
}

void
set_listen(std::string a, std::string b, void* d) {
	auto addr = reinterpret_cast<cpl::net::IP*>(d);
	addr->set(b);
}

void
set_listen_port(std::string a, std::string b, void* d) {
	auto listen_port = reinterpret_cast<int*>(d);
	*listen_port = atoi(b.c_str());
}

void
add_peers(std::string a, std::string b, void* d) {
	auto peers = reinterpret_cast<std::vector<std::string>*>(d);

	std::istringstream ss(b);
	std::string token;

	while (std::getline(ss, token, ',')) {
		peers->push_back(token);
	}

	for (auto p: *peers) {
		std::cout << "added peer: " << p << std::endl;
	}
}
