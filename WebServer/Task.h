// @Author Lin Tao
// @Email putaopu@qq.com
#pragma once
//task��һ��fd�󶨣���û���κ�����ָ��ָ��taskʱ��task��������������ʱ��
//��myEpoll��ж�ض�Ӧ��fd��
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

class Task: public std:: enable_shared_from_this<Task>//,noncopyable
{
public:
	Task(int, std::shared_ptr<MyEpoll>, size_t, std::string, std::shared_ptr<TimerManager>,__uint32_t);
	//Task(size_t, std::shared_ptr<TimerManager>);
	~Task();
	void set_singleManager(std::shared_ptr<SingleTimer> _sp_timer);
	std::shared_ptr<SingleTimer>get_singleManager(){
		return timer.lock();
	}
	//��д��SingleTimer����̳е�reset()
	void reset();
	int get_fd();
	__uint32_t get_events();
	int receive();
	void separate();
	std::shared_ptr<TimerManager> get_timerManager(){
		return timerManager;
	}

	std::shared_ptr<MyEpoll> get_myEpoll(){
		return myEpoll;
	}
private:

	Task(const Task& _task) = delete;
	Task& operator= (const Task& p) = delete;
	ssize_t readn(void* _buff,size_t _n);
	ssize_t writen(void* _buff,size_t _n);
	void state_machine();
	void parse_uri();
	void parse_headers();
	void analysis();
	void recv_body();
	void handle_error(int _errNum, std::string _shortMsg);
	void parse_post();
	void parse_from_urlencoded();
	bool user_verify(const std::string& _name, const std::string& _pwd, bool _isLogin);

	int conver(char ch);

	//void parse_post();
	//void parse_from_urlencoded();


	int againTimes;
	std::string path;
	std::string message;
	std::string fileName;
	std::string postBody;
	int fd;
	int method;
	int httpVersion;
	int readPos;
	bool isFinish;
	bool keep_alive;
	int isError;
	int state;
	int h_state;
	__uint32_t events;//�洢��Ҫ��������Ҫ�������¼�
	std::shared_ptr<MyEpoll> myEpoll;
	//�洢ͷ����Ϣ
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> postMap;
	std::weak_ptr<SingleTimer> timer;
	std::shared_ptr<TimerManager> timerManager;
	
};

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
//	//���Լ�����һ����ʱ����
//
//
//protected:
//	size_t expiredTime;
//	/*
//	* 32λ����:typedef   unsigned int size_t;
//	* 64λ����: typedef  unsigned long size_t;
//	*/
//	bool isDelete;
//	shared_ptr<TimerManager> timerManager;//ָ���ü�ʱ����������ʱ�������
//	inline size_t calcu_time(size_t timeOut);//���³�ʱʱ��
//};