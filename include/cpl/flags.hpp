// This header file provides a command line flag handling
// utility class.
#pragma once

#include <map>        // std::map
#include <string>     // std::string
#include <iostream>   // std::cout
#include <stdexcept>  // exceptions
#include <functional> // std::function

namespace cpl {

class Flags
{
public:
	Flags(std::string name, std::string version)
	: name(name), version(version)
	{
	}

	void add_option(std::string, std::string, std::string,
		std::function<void(std::string, std::string, void*)>);

	void add_option(std::string, std::string, std::string,
		std::function<void(std::string, std::string, void*)>, void*);

	// parses arguments
	void parse(int argc, char* argv[]) throw();

	// Prints usage string message to stdout.
	void print_usage() throw();

private:
	// program name
	std::string name;

	// program version
	std::string version;

	// map from flag to callback
	std::map<std::string, std::function<void(std::string, std::string, void*)>> funcs;

	// map from flag to data pointer
	std::map<std::string, void*> ptrs;

	// map from flag to description string
	std::map<std::pair<std::string, std::string>, std::string> descs;
}; // Flags

inline void Flags :: add_option(std::string long_opt,
	std::string opt, std::string desc,
	std::function<void(std::string, std::string, void*)> cb) {

	// no data pointer
	add_option(long_opt, opt, desc, cb, nullptr);
}

inline void Flags :: add_option(std::string long_opt,
	std::string opt, std::string desc,
	std::function<void(std::string, std::string, void*)> cb,
	void* p) {

	if (opt.length() > 2) {
		// length includes the hyphen
		throw std::invalid_argument("short option longer than one character: " + opt);
	}

	if (funcs.find(long_opt) != funcs.end()) {
		throw std::invalid_argument("redefined flag: " + long_opt);
	}

	if (funcs.find(opt) != funcs.end()) {
		throw std::invalid_argument("redefined flag: " + opt);
	}

	funcs[long_opt] = cb;
	ptrs[long_opt] = p;

	if (opt != "") {
		funcs[opt] = cb;
		ptrs[opt] = p;
	}

	descs[std::pair<std::string, std::string>(opt, long_opt)] = desc;
}

inline void Flags :: print_usage() throw() {
	std::cout << name << " version " << version << " usage:" << std::endl << std::endl;
	for (const auto &i: descs) {
		// short and long options combined
		std::string opts = "";

		if (i.first.first != "") {
			opts = i.first.first + ", " + i.first.second;
		} else {
			opts = "    " + i.first.second;
		}

		printf("  %-25s %s\n", opts.c_str(), i.second.c_str());
	}
}

inline void Flags :: parse(int argc, char* argv[]) throw() {
	auto is_first = true;
	std::string arg;

	for (int i = 0; i < argc; i++) {
		if (!is_first) {
			auto cb = funcs.find(arg);
			if (argv[i][0] == '-') {
				// Start of a new flag.
				cb->second(arg, "", ptrs.find(arg)->second);
				arg = "";
				break;
			}

			if (cb != funcs.end()) {
				// Call callback
				cb->second(arg, argv[i], ptrs.find(arg)->second);
			}

			is_first = true;
			arg = "";

			continue;
		}

		is_first = true;
		arg = "";

		auto cb = funcs.find(argv[i]);
		if (cb != funcs.end()) {
			if (is_first) {
				arg = cb->first;
				is_first = false;
				continue;
			}
		} else {
			// Check for an equals.
			auto argv_str = std::string(argv[i]);
			std::size_t pos = argv_str.find("=");
			if (pos != std::string::npos) {
				// Found an '='.
				auto arg = argv_str.substr(0, pos);
				auto val = argv_str.substr(pos+1);

				auto cb = funcs.find(arg.c_str());
				if (cb != funcs.end()) {
					cb->second(arg.c_str(), val, ptrs.find(arg.c_str())->second);
				}
			}
		}
	}

	if (!is_first) {
		auto cb = funcs.find(arg);
		if (cb != funcs.end()) {
			cb->second(arg, "", ptrs.find(arg)->second);
		}
	}
}

} // cpl
