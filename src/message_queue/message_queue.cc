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

template <class R, class P>
bool
Message_Queue :: pop_with_timeout(Message* m, std::chrono::duration<R,P> d) {
	std::unique_lock<std::mutex> lk(mutex);
	if (cv.wait_for(lk, d, [this]() {
			return queue.size() > 0;	
		})) {
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
