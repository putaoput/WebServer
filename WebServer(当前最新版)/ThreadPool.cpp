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

ThreadPoolTask::ThreadPoolTask()
{	
	args = nullptr;
}

ThreadPoolTask:: ~ThreadPoolTask() {}


void ThreadPoolTask::reset() {
	//STL中智能指针的reset方法内部实现就是新建一个空的智能指针，然后交换他们所指的对象。
	args.reset();
}

void ThreadPoolTask::swap(ThreadPoolTask& _newTask) {
	args.swap(_newTask.args);	
}


//-----------------------------------------------ThreadPool-----------------------------
	std::vector<pthread_t> ThreadPool::threads;
	LT::thread_safe_vector<shared_ptr<ThreadPoolTask>, MAX_QUEUE> ThreadPool::taskQueue;
	int ThreadPool::threadCount = 0;
	bool ThreadPool::isValid = false;
	atomic<int> started = 0;
	

int ThreadPool::create(int _threadCount)//int _threadCount, int _queueSize) 
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
	for (int i = 0; i < _threadCount; ++i) {
		if (pthread_create(&threads[i], NULL, running, (void*)(0)) != 0)
		{
			return -1;
		}
		++started;
		++threadCount;
	}

	isValid = true;
	return 0;
}

int ThreadPool::add(shared_ptr<ThreadPoolTask> _newTask) {
	while (taskQueue.push_back(_newTask) == false)
	{
		yield();//如果填充失败，说明现在队列已经满了，应该把资源交给消费者
	}
	LOG_INFO("Add a new task of fd:%d to ThreadPool, count = %d", _newTask->args->get_fd(),count);
	return 0;
}

void* ThreadPool::running(void* args) {
	while (true) 
	{
		shared_ptr<ThreadPoolTask> SPtask = get_thread_task();	
		if((SPtask->fun)(SPtask->args) == -1)
		{
			//有一种情况是短连接，剩余时错误和重试次数超过上限		
			SPtask->args->get_myEpoll()->del(SPtask->args);	
			LOG_DEBUG("Thread solve a task of client:%d ", SPtask->args->get_fd());		
		}
	}	

	//如果退出线程
	--started;
	pthread_exit(NULL);
	LOG_INFO("pthread exit");
	return (NULL);
}

bool ThreadPool::is_valid() {
	return isValid;
}


shared_ptr<ThreadPoolTask> ThreadPool::get_thread_task(){		
	shared_ptr<ThreadPoolTask> task;
	while (taskQueue.pop_front(task) == false && !shutdwon)
	{
		yield();//如果获取任务失败，说明现在队列已经为空，应该把资源让给生产者
	}
		//检测到关闭信号则关闭
	if ((shutdwon == IMMEDIATE_SHUTDOWN) ||
		(shutdwon == WAIT_SHUTDOWN && !count)) {
		return nullptr;
	}

	LOG_DEBUG("Thread get a task of client:%d, now: head = %d, count = %d", task->args->get_fd(), head,count);
	return task;
}

