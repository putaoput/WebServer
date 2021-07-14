// @Author Lin Tao
// @Email putaopu@qq.com

//第四版重新设计的线程池，设计并实现了无锁任务队列，
//使用了自己写的库:LTSimpleSTL，
//把任务类的仿函数成员对象改成了static，减少了内存占用
#include "ThreadPool.h"
#include "config.h"
#include <iostream>
#include "Log.h"
#include "Mime.h"
#include "MyEpoll.h"

using namespace std;

//------------------------------------------------ThreadPoolTask----------------------------------
//静态成员类外初始化
std::function<int(std::shared_ptr<Task>)> ThreadPoolTask::fun = [](shared_ptr<Task> _task)->int {
	return _task->receive();
};
ThreadPoolTask::ThreadPoolTask(shared_ptr<Task> _task)
	:args(_task){}

ThreadPoolTask:: ~ThreadPoolTask() {}


// 、__is_trivially_copyable

// 功能：判断一个类是否可复制的平凡类
// 条件（同时满足）：
// 1、无显式的无参构造函数
// 2、无自定义的复制构造函数（但可自定义有default修饰的复制构造函数）
// 3、无移动构造函数
// 4、无赋值运算符函数
// 5、无虚函数
// 6、无虚基类
void ThreadPoolTask::reset() {
	//STL中智能指针的reset方法内部实现就是新建一个空的智能指针，然后交换他们所指的对象。
	args.reset();
}

void ThreadPoolTask::swap(ThreadPoolTask& _newTask) {
	args.swap(_newTask.args);	
}


//-----------------------------------------------ThreadPool-----------------------------
	std::vector<pthread_t> ThreadPool::threads;
	LT::thread_safe_vector<size_t, MAX_QUEUE> ThreadPool::taskQueueIdx;
	std::vector<shared_ptr<ThreadPoolTask>> ThreadPool::taskQueue(MAX_QUEUE);
	int ThreadPool::threadCount = 0;
	bool ThreadPool::isValid = false;
	int ThreadPool::shutdown = 0;
	

int ThreadPool::create(int _threadCount)
{
	LOG_DEBUG("ThreadPoll constuct!");

	if (_threadCount <= 0 || _threadCount > MAX_THREADS) {
		printf("num of threads error!!!\n");
		return -1;
	}
	else {
		threads.resize(_threadCount);
	}

	/* 启动线程池里的线程 */
	for (int i = 0; i < _threadCount; ++i)
	{
		if (pthread_create(&threads[i], NULL, running, (void*)(0)) != 0)
		{
			return -1;
		}
		++threadCount;
	}

	isValid = true;
	return 0;
}

int ThreadPool::add(shared_ptr<ThreadPoolTask> _newTask) {
	size_t idx;
	while (taskQueueIdx.push_back(idx) == false)
	{
		//yield();//如果填充失败，说明现在队列已经满了，应该把资源交给消费者
	}
	swap(taskQueue[idx], _newTask);
	LOG_INFO("Add a new task of fd:%d to ThreadPool", _newTask->args->get_fd());
	return 0;
}

void* ThreadPool::running(void* args) {
	while (true) 
	{
		//检测到关闭信号则关闭
		if ((shutdown == IMMEDIATE_SHUTDOWN) ||
			(shutdown == WAIT_SHUTDOWN /*&& taskQueue.empty()*/))
		{
			return nullptr;
		}
		 shared_ptr<ThreadPoolTask> SPtask = get_thread_task();	
		if((SPtask->fun)(SPtask->args) == -1)
		{
			//有一种情况是短连接，剩余时错误和重试次数超过上限		
			SPtask->args->get_myEpoll()->del(SPtask->args);	
			LOG_DEBUG("Thread solve a task of client:%d ", SPtask->args->get_fd());		
		}
	}	

	//如果退出线程
	pthread_exit(NULL);
	LOG_INFO("pthread exit");
	return (NULL);
}

bool ThreadPool::is_valid() {
	return isValid;
}


shared_ptr<ThreadPoolTask> ThreadPool::get_thread_task(){		
	size_t idx = 0;	
	while (taskQueueIdx.pop_front(idx) == false && !shutdown)
	{
		//yield();//如果获取任务失败，说明现在队列已经为空，应该把资源让给生产者
	}
	shared_ptr<ThreadPoolTask> task = taskQueue[idx];
	taskQueue[idx] = nullptr;
	LOG_DEBUG("Thread get a task of client:%d", task->args->get_fd());
	return task;
}

