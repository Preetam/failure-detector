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
	pop()
	{
		std::unique_lock<std::mutex> _(_mutex);
		_cv.wait(_, [this]() {
			return _queue.size() > 0;
		});
		Message m = _queue.front();
		_queue.pop();
		return m;
	}

	void
	push(Message m)
	{
		std::unique_lock<std::mutex> _(_mutex);
		_queue.push(m);
		_cv.notify_one();
	}

private:
	std::mutex _mutex;
	std::condition_variable _cv;
	std::queue<Message> _queue;
};
