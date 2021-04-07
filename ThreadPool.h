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
//#include "base/Thread.h"
//#include "EventLoop.h"

class ThreadPoolTask {
public:
	ThreadPoolTask(std::shared_ptr<Task> _task);
	ThreadPoolTask();
	~ThreadPoolTask();

	//ThreadPoolTask(const ThreadPoolTask& _tpt){}

	void reset();
	void swap(ThreadPoolTask& _newTask);

	std::function<int(std::shared_ptr<Task>)> fun;
	std::shared_ptr<Task> args;
	//fun�����洢һ���ɵ��ö���void����ֵ������������
};

//class MyThread :noncopyable
//{
//public:
//	MyThread();
//	~MyThread();
//	SP_EventLoop start_loop();
//
//private:
//	void thread_running();
//	SP_EventLoop SPloop;
//	Thread thread;
//	MutexLock Tlock;
//	Condition Tnotify;
//};

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

