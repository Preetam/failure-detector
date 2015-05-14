#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

// cpl::Flags
#include <cppl/include/flags.hpp>

// cpl::net::IP
#include <cppl/include/ip.hpp>

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
