// @Author Lin Tao
// @Email putaopu@qq.com
#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <pthread.h>

#include"MutexLock.h"

#include "Task.h"
#include "Condition.h"

class ThreadPoolTask {
public:
	ThreadPoolTask(std::shared_ptr<Task> _task);
	ThreadPoolTask();
	~ThreadPoolTask();

	void reset();
	void swap(ThreadPoolTask& _newTask);

	std::function<int(std::shared_ptr<Task>)> fun;
	std::shared_ptr<Task> args;
	//fun用来存储一个可调用对象void返回值，（参数）。
};


class ThreadPool:noncopyable
{
public:
	static int create(int _threadConut, int _queueSize);
	static int add(std::shared_ptr<ThreadPoolTask> _newTask);
	//static void shutdown_pool(int _shutdown);
	static bool is_valid();
	static void* running(void* args);
	
private:
 	static std::shared_ptr<ThreadPoolTask> get_thread_task();

	static MutexLock TPlock;
	static Condition TPnotify;
	static std::vector<pthread_t> threads;
	static std::vector<std::shared_ptr<ThreadPoolTask>> taskQueue;
	static int threadCount;
	static int queueSize;
	static int head;
	static int tail;
	static int count;
	static int shutdwon;
	static int started;
	static bool isValid;
};