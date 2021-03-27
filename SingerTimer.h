// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once
#include <memory>
#include "config.h"

class TimerManager;
class Task;

class SingleTimer:public std::enable_shared_from_this<SingleTimer>
{
public:
	SingleTimer( size_t _timeOut);
	 ~SingleTimer();
	void reset(size_t _timeOut = TIME_OUT);
	bool is_valid();
	void push_to();
	//用来测试task是否分离成功
	bool separate_success()const{
		return !task;
	}
	void set_task(std::shared_ptr<Task> _task) {
		task = _task;
	}

	void separate_task() {
		task.reset();
	}
	size_t get_expiredTime();
	//把自己放入一个计时器内
	

protected:
	size_t expiredTime;
	/*
	* 32位机器:typedef   unsigned int size_t;
	* 64位机器: typedef  unsigned long size_t;
	*/
	bool isDelete;
	//bool isValid;
	//std::shared_ptr<TimerManager> timerManager;//指定该计时对象所属的时间管理器
	inline size_t calcu_time(size_t timeOut = 0);//更新超时时间
	std::shared_ptr<Task> task;
};
