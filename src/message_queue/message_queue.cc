#include "message_queue.hpp"

Message
Message_Queue :: pop() {
	std::unique_lock<std::mutex> lk(mutex);
	// Wait until there is a message.
	cv.wait(lk, [this]() {
		return queue.size() > 0;
	});
	Message m = queue.front();
	queue.pop();
	return m;
}

bool
Message_Queue :: pop_with_timeout(Message* m, int milliseconds) {
	std::unique_lock<std::mutex> lk(mutex);
	// Wait until there is a message or we timeout.
	if (cv.wait_for(lk, std::chrono::milliseconds((long)milliseconds),
		[this]() { return queue.size() > 0; })) {
		Message message = queue.front();
		queue.pop();
		*m = message;
		return true;
	}
	// Timed out
	return false;
}

void
Message_Queue :: push(Message m) {
	std::unique_lock<std::mutex> lk(mutex);
	queue.push(m);
	cv.notify_one();
}

size_t
Message_Queue :: size() {
	std::unique_lock<std::mutex> lk(mutex);
	return queue.size();
}
