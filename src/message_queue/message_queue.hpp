#pragma once

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

	void
	push(Message m);

private:
	std::mutex mutex;
	std::condition_variable cv;
	std::queue<Message> queue;
}; // Message_Queue
