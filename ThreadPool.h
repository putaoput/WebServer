// @Author Lin Tao
// @Email putaopu@qq.com
#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <pthread.h>


#include "Task.h"

class ThreadPoolTask {
public:
	ThreadPoolTask(std::shared_ptr<Task> _task);
	ThreadPoolTask();
	void reset();
	void swap(ThreadPoolTask& _newTask);

	std::function<void(std::shared_ptr<Task>)> fun;
	std::shared_ptr<Task> args;
	//fun�����洢һ���ɵ��ö���void����ֵ������������
};

class ThreadPool
{
public:
	ThreadPool(int _threadConut, int _queueSize);
	static int add(ThreadPoolTask);
	//static void shutdown_pool(int _shutdown);
	static bool is_valid();
	static void* running(void* args);

private:
	static pthread_mutex_t lock;
	static pthread_cond_t notify;
	static std::vector<pthread_t> threads;
	static std::vector<ThreadPoolTask> taskQueue;
	static int threadCount;
	static int queueSize;
	static int head;
	static int tail;
	static int count;
	static int shutdwon;
	static int started;
	static bool isValid;
};

