#pragma once

#include <cstdint>

void write8be(uint8_t, uint8_t*);
void write16be(uint16_t, uint8_t*);
void write32be(uint32_t, uint8_t*);
void write64be(uint64_t, uint8_t*);

inline void
write8be(uint8_t v, uint8_t* dest) {
	*dest = v;
}

inline void
write16be(uint16_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

inline void
write32be(uint32_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

inline void
write64be(uint64_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

