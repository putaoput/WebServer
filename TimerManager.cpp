// @Author Lin Tao
// @Email putaopu@qq.com

#include <sys/time.h>
#include <iostream>

#include "SingerTimer.h"
#include "TimerManager.h"
#include "MyEpoll.h"

using namespace std;

//---------------SingleTimer的实现------------------


SingleTimer::SingleTimer(shared_ptr<TimerManager> _timerManager, size_t _timeOut):
	isDelete(false),//isValid(true)
	timerManager(_timerManager)
{
	expiredTime = calcu_time(_timeOut);
}

SingleTimer::~SingleTimer(){}

void SingleTimer:: reset(size_t _timeOut) {
	expiredTime = calcu_time(_timeOut);
}



bool SingleTimer::is_valid() {
	return expiredTime >= calcu_time();
}

void SingleTimer::push_to() {
	//先检查智能指针是否为空
	if (timerManager == nullptr) {
		perror("the timerManager point to nullptr!!!\n");
	}
	shared_ptr<SingleTimer> spTimer = shared_from_this();
	#ifdef TEST
		cout << " push to!!!" << endl;
		cout << spTimer.use_count() << endl;
		cout << timerManager.use_count() << endl;
		
	#endif
	timerManager->add(spTimer);
	#ifdef TEST
		cout << "end push_to" << endl;
	#endif
}

size_t SingleTimer::get_expiredTime() {
	return expiredTime;
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
	:isValid(false)
{
	if (pthread_mutex_init(&lock,NULL) == -1) {
		perror("lock of TimerManager init failed!!!\n");
		return;
	}
	if (pthread_cond_init(&notify, NULL) == -1) {
		perror("notify of TimerManager init failed!!!\n");
		return;
	}
	isValid = true;
}

void TimerManager::add(shared_ptr<SingleTimer> _singleTimer) {

	#ifdef TEST
		cout << "SingleTimer add!!" << endl;
	#endif

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("lock failed while add singleaTimer!!");
	}
	timerManager.push(_singleTimer);
	if (pthread_cond_signal(&notify) != 0)
	{
		perror("pthread_cond_signal failed after timerManager pop!!!\n");
		return ;
	}

	if (pthread_mutex_unlock(&lock) != 0) {
		perror("pthread_mutex_unlock failed after after timerManager pop!!!\n");
		return ;
	}
}

void TimerManager::pop() {
	if (pthread_mutex_lock(&lock) != 0) {
		perror("lock failed while pop timer!!!\n");
		return ;
	}
	while (timerManager.empty() == false && timerManager.top()->is_valid() == false)
		{
			timerManager.pop();
			#ifdef TEST
				cout << "pop a task" << endl;
			#endif
		}

	//唤醒等待该互斥量的线程
	if (pthread_cond_signal(&notify) != 0)
	{
		perror("pthread_cond_signal failed after timerManager pop!!!\n");
		return ;
	}

	if (pthread_mutex_unlock(&lock) != 0) {
		perror("pthread_mutex_unlock failed after after timerManager pop!!!\n");
		return ;
	}
	//这里有一个巧妙设计在于继承了之后，可以很好避免两次判断。
}

bool TimerManager::is_valid() {
	return isValid;
}

//---------------TimerManeger里时间堆内比较函数的实现------------------

bool  PSingleTimerCmp ::operator()( const shared_ptr<SingleTimer> spA,const shared_ptr<SingleTimer> spB) const {
	return spA->get_expiredTime() > spB->get_expiredTime();
}
	//如果指针前面加const代表指针指向的内容不可以修改，const在指针后面代表指针本身的内容不可以修改
