// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once

#include <queue>
#include <memory> //��������ָ��

#include "config.h"
#include "Task.h"
#include "SingerTimer.h"
#include "MutexLock.h"
#include "Condition.h"

//������һ�����ȶ��У��󶥶�
class PSingleTimerCmp {
public:
	bool operator()(const std::shared_ptr<SingleTimer> spA, const std::shared_ptr<SingleTimer> spB) const;
};

class TimerManager:noncopyable
{
public:
	TimerManager();
	void add(std::shared_ptr<SingleTimer> _singleTimer);
	void pop();
	bool is_valid();


private:
	MutexLock TMlock;
	Condition  TMnotify;
	bool isValid;
	std::priority_queue < std::shared_ptr<SingleTimer>, std::vector<std::shared_ptr<SingleTimer>>, PSingleTimerCmp> timerManager;
};





