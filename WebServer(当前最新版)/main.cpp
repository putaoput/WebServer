// @Author Lin Tao
// @Email putaopu@qq.com

#include <signal.h>
#include <iostream>
#include <string.h>

#define __DEBUG__ //��������ģʽ����������ԵĻ�����ֱ��ע�͵���һ��

#include "config.h"
#include "MyEpoll.h"
#include "Task.h"
#include "ThreadPool.h"
#include "TimerManager.h"
#include "Log.h"
#include "SqlConnPool.h"
#include "Mime.h"

using namespace std;

//namespace{
//----------------------SqlConnPool-------------
//�˴����ó�ʼ��MySql���ӳص�һЩĬ�ϲ���
//const char* SERVER_HOST = "localHost";
//constexpr int SERVER_SQLPORT = 3306;
//const char* SERVER_SQLUSER = "root";
//const char* SERVER_SQLPWD = "root";
//const char* SERVER_DBNAME = "myServer";
//constexpr int SERVER_SQLCOONPOOL_NUM = 1024;
//}


int main(int argc, char* argv[]) {

	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL)) {
		return -1;
	}
	cout << 1;
	//�����½���־�������¼���������������
	Log::instance()->init(0, "./Log", "Log", 1024);
	//���ӵ����ݿ�
	//SqlConnPool::instance()->init(SERVER_HOST, SERVER_SQLPORT, SERVER_SQLUSER, SERVER_SQLPWD, SERVER_DBNAME, SERVER_SQLCOONPOOL_NUM);
	SqlConnPool::instance()->init("localhost", 3306, "lintao", "123456", "myServer",1024);

	//��ʼ��һЩ������
	MimeType::instance()->init();
	WebPage::instance()->init();
	cout << 2;	
	//��������Ĳ����� -p port, 
	if (argc == 1) {
		cout << "the first arg is port_num, the second is pthread num,please divided by black:" << endl;
		return -1;
	}
	if (argc < 3) {
		cout << "Please enter the port num and phread num" << endl;
		return -1;
	}

	do {
		 ThreadPool::create(atoi(argv[2]));
		if (!ThreadPool::is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}
		cout << 3;
		//�½�һ����ʱ�����������г�ʱ����
		shared_ptr<TimerManager> timerManager(new TimerManager);
		if (!timerManager->is_valid()) {
			cout << "timerManager init failed!!!\n" << endl;
			return -1;
		}
		cout << 4;
		//�½�һ��epoll������Ϊreactorģ�͵����̣߳��������еĶ�д�����¼�
		shared_ptr<MyEpoll> myEpoll(new MyEpoll(atoi(argv[1]), timerManager));
		if (!myEpoll->is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}
		cout << 5;
		//Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
		LOG_INFO("The WebServer init");
		LOG_INFO("Port:%d", atoi(argv[1]),"           Thread num:%d",atoi(argv[2]));

	//����
		while (true) {
			cout << 6;
			myEpoll->wait(MAX_EVENATS, -1, PATH);
			timerManager->pop();//����ʱ�¼�,����������ɰ����ȶ��з�װ��֮��,��д��pop
		}
	} while (false)
	LOG_ERROR("Sever init failed!");
	SqlConnPool::instance()->close_pool();//�ر����ݿ�

	return 0;
}

