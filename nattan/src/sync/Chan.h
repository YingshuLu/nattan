/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_CHAN_H
#define NATTAN_SYNC_CHAN_H

#include <memory>
#include "base/Uncopyable.h"
#include "sync/TaskMutex.h"
#include "sync/TaskCond.h"
#include "sync/LockGuard.h"

namespace nattan {

template<typename T>

class Chan {
public:
	Chan(const unsigned int cap = 64):
	 capacity(cap), notFull(mutex), notEmpty(mutex ), closed(false) {
		messages = new T[capacity];
		putIndex = 0;
		takeIndex = 0;
		count = 0;
	}

	bool isFull() { return count == capacity; }
	bool isEmpty() { return count == 0; }

	int size() { return count; }

	void send(T& msg) {
		LockGuard guard(mutex);
		while(isFull()) {
			notFull.wait();
		}
		enqueue(msg);
	}

	bool sendTill(T& msg, time_t ms) {
		LockGuard guard(mutex);
		if(isFull()) {
			notFull.waitTill(ms);
			if (isFull()) return false;
		}
		enqueue(msg);
		return true;
	}

	 T recv() {
		LockGuard guard(mutex);
		while(isEmpty()) {
			notEmpty.wait();
		}
		return dequeue();
	}

	bool recvTill(T& msg, time_t ms) {
		LockGuard guard(mutex);
		if(isEmpty()) {
			notEmpty.waitTill(ms);
			if (isEmpty()) return false;
		}
		msg = dequeue();
		return true;
	}

	bool isClosed() {
		return closed;
	}

	~Chan() {
		delete[] messages;
	}

	int close() {
		count = 0;
		putIndex = 0;
		takeIndex = 0;
		closed = true;
	}

private:
	void enqueue(T& e) {
		messages[putIndex] = e;
		if (++putIndex == capacity) putIndex = 0;
		count++;
		notEmpty.signalAll();
	}

	T dequeue() {
		T& e = messages[takeIndex];
		if(++takeIndex == capacity) takeIndex = 0;
		count --;
		notFull.signalAll();
		return e;
	}

private:

	bool closed;
	T* messages;
	TaskMutex mutex;
	TaskCond notFull;
	TaskCond notEmpty;

	volatile unsigned int capacity;
	volatile unsigned int putIndex;
	volatile unsigned int takeIndex;
	volatile unsigned int count;
};


} // namespace nattan


#endif
