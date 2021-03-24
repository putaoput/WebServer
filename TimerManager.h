// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once

#include <queue>
#include <memory> //引入智能指针

#include "config.h"
#include "Task.h"
#include "SingerTimer.h"

//本质是一个优先队列，大顶堆
class PSingleTimerCmp {
public:
	bool operator()(const std::shared_ptr<SingleTimer> spA, const std::shared_ptr<SingleTimer> spB) const;
};

class TimerManager
{
public:
	TimerManager();
	void add(std::shared_ptr<SingleTimer> _singleTimer);
	void pop();
	bool is_valid();


private:
	pthread_mutex_t lock;
	pthread_cond_t  notify;
	bool isValid;
	std::priority_queue < std::shared_ptr<SingleTimer>, std::vector<std::shared_ptr<SingleTimer>>, PSingleTimerCmp> timerManager;
};





