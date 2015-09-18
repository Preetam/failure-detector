#include "catch.hpp"
#include <cstring>

#include "encoding.h"

TEST_CASE("encoding", "[encoding]") {
	uint8_t data[2];
	write16be(0x1234, data);
	uint8_t expected[2] = {0x34, 0x12};
	REQUIRE(memcmp(data, expected, 2) == 0);
}