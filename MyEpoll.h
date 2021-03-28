// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include "ThreadPool.h"
class TimeManager;
class MyEpoll
{
public:
	MyEpoll(int _port, std::shared_ptr<TimerManager> _timerManager);
	bool is_valid();
	int add(std::shared_ptr<Task> _task);
	int mod(std::shared_ptr<Task> _task);
	int del(std::shared_ptr<Task> _task);
	int wait(int _maxEvents, int timeOut, std::string _path);

	void add_to_epollTask(std::shared_ptr<Task> _task){
		epollTask[_task->get_fd()] = _task;
	}
private:
	int start(int _port);
	int set_sock_non_block(int _listen_fd);
	int accept_connection(int _fd, size_t timeOut, std::string _path);

	int listen_fd;
	int epoll_fd;
	epoll_event* eventsArr;
	bool isValid;//初始化失败，则isvalid会变成fasle;
	std::unordered_map<int, std::shared_ptr<Task>> epollTask;
	std::vector<std::shared_ptr<Task>> TaskArr;
	std::shared_ptr<TimerManager> timerManager;//时间管理器
};

