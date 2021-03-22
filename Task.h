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

	//��д��SingleTimer����̳е�reset()
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
	__uint32_t events;//�洢��Ҫ��������Ҫ�������¼�
	std::shared_ptr<MyEpoll> myEpoll;
	//�洢ͷ����Ϣ
	std::map<std::string, std::string> headers;

	
};

//ճ�������������������̳���

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

/*
c++�е�ͷ�ļ�ѭ������
����

    ����Ŀ�ļ����ʱ��������֯�����������׳���ͷ�ļ���ѭ������
    ��ʱ����Ȼû�г���ѭ�����ã�����ͷ�ļ��ദ��include�����±����ٶȱ���

����취

�ʵ���ʹ��ǰ������

    ʲô��ǰ������

      ���ǵ���ֻ�õ�ĳ�����͵�ָ���������ʱ�����Բ��ð��������͵�ͷ�ļ�include������ֻ��Ҫ����������ͼ���

    why

      ��Ϊ��ֻ��Ҫʹ��������ָ�������ʱ��������������ļ�ʱ�Ͳ���Ҫ֪��������͵��ڴ沼�֡�������ֻ��Ҫ��ָ��������÷���ɵ�ַ���ɡ�
*/
