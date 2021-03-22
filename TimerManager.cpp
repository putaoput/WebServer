// @Author Lin Tao
// @Email putaopu@qq.com

#include <sys/time.h>
#include <iostream>

#include "SingerTimer.h"
#include "TimerManager.h"
#include "MyEpoll.h"

using namespace std;

//---------------SingleTimer��ʵ��------------------


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
	//�ȼ������ָ���Ƿ�Ϊ��
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
	//now�����������������ֱ����������΢�����
	//����������ʹ�ú��������㳬ʱʱ���
	return (((now.tv_sec % 10000)* 1000) + (now.tv_usec / 1000)) + _timeOut;
}

//---------------TimerManeger��ʵ��------------------
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
		}

	//���ѵȴ��û��������߳�
	if (pthread_cond_signal(&notify) != 0)
	{
		perror("pthread_cond_signal failed after timerManager pop!!!\n");
		return ;
	}

	if (pthread_mutex_unlock(&lock) != 0) {
		perror("pthread_mutex_unlock failed after after timerManager pop!!!\n");
		return ;
	}
	//������һ������������ڼ̳���֮�󣬿��Ժܺñ��������жϡ�
}

bool TimerManager::is_valid() {
	return isValid;
}

//---------------TimerManeger��ʱ����ڱȽϺ�����ʵ��------------------

bool  PSingleTimerCmp ::operator()( const shared_ptr<SingleTimer> spA,const shared_ptr<SingleTimer> spB) const {
	return spA->get_expiredTime() > spB->get_expiredTime();
}
	//���ָ��ǰ���const����ָ��ָ������ݲ������޸ģ�const��ָ��������ָ�뱾�������ݲ������޸�