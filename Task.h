// @Author Lin Tao
// @Email putaopu@qq.com
#pragma once

#include <sys/epoll.h>
#include <string>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <unistd.h>
#include <sys/mman.h>


//#include "MyEpoll.h"
//#include "TimerManager.h"
#include "Mime.h"
#include "SingerTimer.h"

class MyEpoll;
class TimerManager;

class Task: public SingleTimer
{
public:
	Task(int, std::shared_ptr<MyEpoll>, size_t, std::string, std::shared_ptr<TimerManager>,__uint32_t);
	//Task(size_t, std::shared_ptr<TimerManager>);
	~Task();

	//重写从SingleTimer里面继承的reset()
	void reset();
	int get_fd();
	__uint32_t get_events();
	int receive();
private:
	ssize_t readn(char* _buff);
	ssize_t writen(char* _buff,size_t _n);
	void state_machine();
	void parse_uri();
	void parse_headers();
	void analysis();
	void recv_body();
	void handle_error(int _errNum, std::string _shortMsg);

	int againTimes;
	std::string path;
	std::string message;
	std::string fileName;
	int fd;
	int method;
	int httpVersion;
	int readPos;
	bool isFinish;
	bool keep_alive;
	int isError;
	int state;
	int h_state;
	__uint32_t events;//存储需要该任务需要监听的事件
	std::shared_ptr<MyEpoll> myEpoll;
	//存储头部信息
	std::map<std::string, std::string> headers;

	
};

//粘贴过来方便分析子类与继承类

//class SingleTimer
//	: public std::enable_shared_from_this<SingleTimer>
//{
//public:
//	SingleTimer(shared_ptr<TimerManager> _timerManager, size_t _timeOut = TIME_OUT);
//	virtual ~SingleTimer();
//	virtual void reset(size_t _timeOut = TIME_OUT);
//	bool is_valid();
//	void push_to();
//	size_t get_expiredTime();
//	//把自己放入一个计时器内
//
//
//protected:
//	size_t expiredTime;
//	/*
//	* 32位机器:typedef   unsigned int size_t;
//	* 64位机器: typedef  unsigned long size_t;
//	*/
//	bool isDelete;
//	shared_ptr<TimerManager> timerManager;//指定该计时对象所属的时间管理器
//	inline size_t calcu_time(size_t timeOut);//更新超时时间
//};

/*
c++中的头文件循环引用
问题

    在项目文件变多时，由于组织不当，很容易出现头文件的循环引用
    有时候虽然没有出现循环引用，但是头文件多处被include，导致编译速度变慢

解决办法

适当的使用前置声明

    什么是前置声明

      就是当你只用到某个类型的指针或者引用时，可以不用把整个类型的头文件include进来。只需要声明这个类型即可

    why

      因为在只需要使用这个类的指针和引用时，编译器编译此文件时就不需要知道这个类型的内存布局。编译器只需要把指针或者引用翻译成地址即可。
*/
