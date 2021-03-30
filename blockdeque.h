//@Author Lin Tao
//@Email putaopu@qq.com

#pragma once
//这个头文件主要是使用模板编程的方法，给标准库里的deque封装了一层，用消费者
//和生产者的思路保证了线程安全。
//#include <mutex>
//#include <condition_variable>//这是c++11提供的

#include <deque>
#include <sys/time.h>

#include "MutexLock.h"
#include "Condition.h"

template<class T>
class BlockDeque {
public:
	explicit BlockDeque(size_t _maxCapacity = 1000);

	~BlockDeque();

	void clear();

	bool empty();

	bool full();

	void close();

	size_t size();

	size_t capacity();

	T front();

	T back();

	void push_back(const T& _item);

	void push_front(const T& _item);

	bool pop(T& _item);

	bool pop(T& _item, int _timeOut);

	void flush();
private:
	std::deque<T> deq;
	size_t Qcapacity;
	//std::mutex mtx;
	MutexLock Qlock;
	Condition consumer;
	Condition producter;
	bool isClose;
	//std::condition_variable condConsumer;
	//std::condition_variable condProducer;
};

template<class T>
BlockDeque<T>::BlockDeque(size_t _maxCapacity)
	:Qcapacity(_maxCapacity),
	Qlock(),
	consumer(Qlock),
	producter(Qlock)
{
	assert(_maxCapacity > 0);
	isClose = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
	close();
};

template<class T>
void BlockDeque<T>::clear() {
	MutexLockGuard lock(Qlock);
	deq.clear();
}

template<class T>
T BlockDeque<T>::front() {
	MutexLockGuard lock(Qlock);
	return deq.front();
}


template<class T>
T BlockDeque<T>::back() {
	MutexLockGuard lock(Qlock);
	return deq.back();
}

template<class T>
size_t BlockDeque<T>::size() {
	MutexLockGuard lock(Qlock);
	return deq.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
	MutexLockGuard lock(Qlock);
	return Qcapacity;
}

template<class T>
void BlockDeque<T>::push_back(const T& _item) {
	MutexLockGuard lock(Qlock);
	while (deq.size() >= Qcapacity) {
		producter.wait();
	}
	deq.push_back(_item);
	consumer.notify();
}

template<class T>
void BlockDeque<T>::push_front(const T& _item) {
	MutexLockGuard lock(Qlock);
	while (deq.size() >= capacity) {
		producter.wait();
	}
	deq.push_front(_item);
	consumer.notify();
}

template<class T>
bool BlockDeque<T>::empty() {
	MutexLockGuard lock(Qlock);
	return deq.empty();
}

template<class T>
bool BlockDeque<T>::full() {
	MutexLockGuard lock(Qlock);
	return deq.size() >= Qcapacity;
}

template<class T>
void BlockDeque<T>::close() {
	MutexLockGuard lock(Qlock);
	deq.clear();
	isClose = true;
	consumer.notifyAll();
	producter.notifyAll();
}
template<class T>
bool BlockDeque<T>::pop(T& item) {
	MutexLockGuard lock(Qlock);
	while (deq.empty()) {
		consumer.wait();
		if (isClose) {
			return false;
		}
	}
	item = deq.front();
	deq.pop_front();
	producter.notify();
	return true;
}

template<class T>
bool BlockDeque<T>::pop(T& _item, int _timeOut) {
	MutexLockGuard lock(Qlock);
	while (deq.empty()) {
		if (consumer.waitForSeconds(_timeOut)) {
			return false;
		}
		if (isClose) {
			return false;
		}
	
	}
	_item = deq.front();
	deq.pop_front();
	producter.notify();
	return true;
}

//flush函数用来唤醒正在等待的消费者操作
template<class T>
void BlockDeque<T>::flush() {
	//MutexLockGuard lock(Qlock);
	consumer.notify();
}

