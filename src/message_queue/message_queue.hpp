#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "../message.hpp"

class Message_Queue
{
public:
	Message_Queue()
	{
	}

	// Disable copying.
	Message_Queue(const Message_Queue&) = delete;
	Message_Queue& operator =(const Message_Queue&) = delete;

	Message
	pop();

	bool
	pop_with_timeout(Message*, int);

	void
	push(Message m);

	size_t
	size();

private:
	std::mutex mutex;
	std::condition_variable cv;
	std::queue<Message> queue;
}; // Message_Queue
