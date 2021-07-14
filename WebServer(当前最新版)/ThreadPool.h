// @Author Lin Tao
// @Email putaopu@qq.com

//���İ���Ʋ�ʵ��������������У�ʹ�����Լ���д�Ŀ⣺LTSimpleSTL
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
	std::shared_ptr<Task> args;//atomic�����޷�������ָ�����ֲ���
	//fun�����洢һ���ɵ��ö���void����ֵ������������
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