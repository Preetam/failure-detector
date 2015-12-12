#pragma once

#include <cstring>

class Buffer
{
public:
	Buffer(int capacity)
	: m_capacity(capacity)
	{
	}

	int
	write(uint8_t* src, int src_len)
	{
		if (m_bytes.size()+src_len >= m_capacity) {
			return -1;
		}
		int written = 0;
		while (m_bytes.size() < m_capacity) {
			m_bytes.push_back(*(src++));
			written++;
			if (--src_len == 0) {
				break;
			}
		}
		return written;
	}

	int
	read(uint8_t* dest, int dest_len)
	{
		int result = peek(dest, dest_len);
		if (result < 0) {
			return result;
		}
		m_bytes.erase(m_bytes.begin(), m_bytes.begin() + result);
		return result;
	}

	int
	peek(uint8_t* dest, int dest_len)
	{
		if (m_bytes.size() < dest_len) {
			return -1;
		}
		memcpy(dest, m_bytes.data(), dest_len);
		return dest_len;
	}

private:
	std::vector<uint8_t> m_bytes;
	int                  m_capacity;

}; // Buffer
