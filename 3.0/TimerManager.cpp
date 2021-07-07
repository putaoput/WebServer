// @Author Lin Tao
// @Email putaopu@qq.com

#include <sys/time.h>
#include <iostream>

#include "SingerTimer.h"
#include "TimerManager.h"
#include "MyEpoll.h"
#include "config.h"

using namespace std;

//---------------SingleTimer的实现------------------

SingleTimer::SingleTimer( size_t _timeOut)
{
	expiredTime = calcu_time(_timeOut);
}

SingleTimer::~SingleTimer(){
	//如果task智能指针不为空，则当task不在任务队列里时，SingleTimer里面是最后两个指向task的智能指针之一
	//还有一个在MyEpoll里面，所以SingleTimer析构掉之后，task随之析构，然后task析构时，要从MyEpoll中卸载对应fd。
	//总而之fd和Task是绑定的
	if(task){
		task->get_myEpoll()->del(task);
	}
}

void SingleTimer:: reset(size_t _timeOut) {
	expiredTime = calcu_time(_timeOut);
}

bool SingleTimer::is_valid() {
	return expiredTime >= calcu_time();
}

bool SingleTimer::is_delete(){
	return expiredTime <= calcu_time();
}

size_t SingleTimer::calcu_time(size_t _timeOut) {
	struct timeval now;
	gettimeofday(&now, NULL);
	//now里面有两个参数，分别是以秒和以微秒计数
	//但是这里是使用毫秒来计算超时时间的
	return (((now.tv_sec % 10000)* 1000) + (now.tv_usec / 1000)) + _timeOut;
}

//---------------TimerManeger的实现------------------
TimerManager::TimerManager() 
	:isValid(false),
	TMlock(),
	TMnotify(TMlock)
{
	/*if (pthread_mutex_init(&lock,NULL) == -1) {
		perror("lock of TimerManager init failed!!!\n");
		return;
	}
	if (pthread_cond_init(&notify, NULL) == -1) {
		perror("notify of TimerManager init failed!!!\n");
		return;
	}*/
	isValid = true;
}

void TimerManager::add(shared_ptr<SingleTimer> _singleTimer) {
#ifdef TEST
	cout << "SingleTimer add!!" << endl;
#endif

	MutexLockGuard lock(TMlock);
	timerManager.push(_singleTimer);
	TMnotify.notify();
}

void TimerManager::pop() {
	MutexLockGuard lock(TMlock);
	while (timerManager.empty() == false && timerManager.top()->is_delete())
		{
			shared_ptr<Task> task = timerManager.top()->get_task();
			if(task){
			task->get_myEpoll()->del(task);
#ifdef _EPOLL_
				cout << "pop a task" << endl;
#endif
			}
			timerManager.pop();
		}
#ifdef TEST
		cout << "timerManager.size = " << timerManager.size() << endl;
#endif
	TMnotify.notify();
}

bool TimerManager::is_valid() {
	return isValid;
}

//---------------TimerManeger里时间堆内比较函数的实现------------------

bool  PSingleTimerCmp ::operator()( const shared_ptr<SingleTimer> spA,const shared_ptr<SingleTimer> spB) const {
	return spA->get_expiredTime() > spB->get_expiredTime();
}
	//如果指针前面加const代表指针指向的内容不可以修改，const在指针后面代表指针本身的内容不可以修改
