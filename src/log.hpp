#pragma once

#ifdef DEBUG 
#define LOG(x) \
	std::cerr << "[" <<  __FILE__ << ":" << __LINE__ << "] " \
	<< x << std::endl;
#else 
#define LOG(x)
#endif
