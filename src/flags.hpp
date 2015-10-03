#pragma once

#include <vector>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>

// cpl::Flags
#include <cpl/flags.hpp>

// cpl::net::IP
#include <cpl/net/ip.hpp>

void
show_help(std::string a, std::string b, void* d) {
	auto flags = reinterpret_cast<cpl::Flags*>(d);
	flags->print_usage();
	exit(0);
}

void
set_listen_string(std::string a, std::string b, void* d) {
	auto str = reinterpret_cast<std::string*>(d);
	*str = b;
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
