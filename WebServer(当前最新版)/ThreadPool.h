// @Author Lin Tao
// @Email putaopu@qq.com

//���İ���Ʋ�ʵ��������������У�ʹ�����Լ���д�Ŀ⣺LTSimpleSTL
#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <pthread.h>

#include "LTSimpleSTL/thread_safe_vector.h"
#include "Task.h"
#include "Condition.h"

class ThreadPoolTask {
public:
	ThreadPoolTask(std::shared_ptr<Task> _task);
	~ThreadPoolTask();
	void reset();
	void swap(ThreadPoolTask& _newTask);

	static std::function<int(std::shared_ptr<Task>)> fun;
	std::shared_ptr<Task> args;
	//fun�����洢һ���ɵ��ö���void����ֵ������������
};


class ThreadPool:noncopyable
{
public:
	static int create(int _threadConut, int _queueSize);
	static int add(std::shared_ptr<ThreadPoolTask> _newTask);
	static void shutdown_pool(int _shutdown);
	static bool is_valid();
	static void* running(void* args);
	
private:
 	static std::shared_ptr<ThreadPoolTask> get_thread_task();

	static MutexLock TPlock;
	static Condition TPnotify;
	static std::vector<pthread_t> threads;
	static LT::thread_safe_vector<std::shared_ptr<ThreadPoolTask>, MAX_QUEUE> taskQueue;
	static int threadCount;
	static int shutdwon;
	static std::atomic<int> started;
	static bool isValid;
};