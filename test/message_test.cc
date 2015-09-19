#include "catch.hpp"
#include <cstring>

#include "message.hpp"

TEST_CASE("message", "[message]") {
	uint8_t data[4] = {0x1, 0x2, 0x3, 0x4};
	Message m;
	m.type = 0x99;
	m.length = 4;
	m.data = data;
	uint8_t packed[10];
	int status = m.pack(packed, 10);
	REQUIRE(status == 0);

	status = m.unpack(packed, 10);
	REQUIRE(status == 0);
	REQUIRE(m.type == 0x99);
	REQUIRE(m.length == 4);
	REQUIRE(memcmp(m.data, data, 4) == 0);
}
