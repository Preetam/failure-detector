#include "message_queue.hpp"

Message
Message_Queue :: pop() {
	std::unique_lock<std::mutex> lk(mutex);
	cv.wait(lk, [this]() {
		return queue.size() > 0;
	});
	Message m = queue.front();
	queue.pop();
	return m;
}

void
Message_Queue :: push(Message m) {
	std::unique_lock<std::mutex> lk(mutex);
	queue.push(m);
	cv.notify_one();
}
