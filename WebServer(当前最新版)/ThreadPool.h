// @Author Lin Tao
// @Email putaopu@qq.com

//第四版设计并实现了无锁任务队列，使用了自己编写的库：LTSimpleSTL
#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <pthread.h>
#include <atomic>

#include "LTSimpleSTL/thread_safe_vector.h"
#include "Task.h"
#include "Condition.h"
#include "config.h"

struct ThreadPoolTask {
	ThreadPoolTask(std::shared_ptr<Task> _task);
	~ThreadPoolTask();
	void reset();
	void swap(ThreadPoolTask& _newTask);

	static std::function<int(std::shared_ptr<Task>)> fun;
	std::shared_ptr<Task> args;//atomic类型无法存智能指针这种不是
	//fun用来存储一个可调用对象void返回值，（参数）。
};

class ThreadPool:noncopyable
{
public:
	static int create(int _threadConut);
	static int add(std::shared_ptr<ThreadPoolTask> _newTask);
	static void shutdown_pool(int _shutdown);
	static bool is_valid();
	static void* running(void* args);
	
private:
 	static std::shared_ptr<ThreadPoolTask> get_thread_task();
	static std::vector<pthread_t> threads;
	static LT::thread_safe_vector<size_t, MAX_QUEUE> taskQueueIdx;
	static std::vector<std::shared_ptr<ThreadPoolTask>> taskQueue;
	static int threadCount;
	static int shutdown;
	static bool isValid;
};