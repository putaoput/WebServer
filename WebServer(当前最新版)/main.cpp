// @Author Lin Tao
// @Email putaopu@qq.com

#include <signal.h>
#include <iostream>
#include <string.h>
//#include <unistd.h> //getopt用来分析命令行参数

#define __DEBUG__ //开启调试模式，如果不调试的话可以直接注释掉这一行

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
//此处设置初始化MySql连接池的一些默认参数
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

	//首先新建日志，方便记录服务器的启动情况
	Log::instance()->init(0, "./Log", "Log", 1024);
	//连接到数据库
	//SqlConnPool::instance()->init(SERVER_HOST, SERVER_SQLPORT, SERVER_SQLUSER, SERVER_SQLPWD, SERVER_DBNAME, SERVER_SQLCOONPOOL_NUM);
	SqlConnPool::instance()->init("localhost", 3306, "lintao", "123456", "myServer",1024);

	//初始化一些单例：
	MimeType::instance()->init();
	WebPage::instance()->init();
		
	//可以输出的参数有 -p port, 
	if (argc == 1) {
		cout << "the first arg is port_num, the second is pthread num,please divided by black:" << endl;
		return -1;
	}
	if (argc < 3) {
		cout << "Please enter the port num and phread num" << endl;
		return -1;
	}

	do {
		ThreadPool::create(atoi(argv[2]), QUEUE_NUM);
		if (!ThreadPool::is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}

		//新建一个计时器，处理所有超时对象
		shared_ptr<TimerManager> timerManager(new TimerManager);
		if (!timerManager->is_valid()) {
			cout << "timerManager init failed!!!\n" << endl;
			return -1;
		}

		//新建一个epoll对象，作为reactor模型的主线程，处理所有的读写就绪事件
		shared_ptr<MyEpoll> myEpoll(new MyEpoll(atoi(argv[1]), timerManager));
		if (!myEpoll->is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}
	
		//Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
		LOG_INFO("The WebServer init");
		LOG_INFO("Port:%d", atoi(argv[1]),"           Thread num:%d",atoi(argv[2]));

	//启动
		while (true) {
			myEpoll->wait(MAX_EVENATS, -1, PATH);
			timerManager->pop();//处理超时事件,这里可以理解成把优先队列封装完之后,重写了pop
		}
	} while (false)
	LOG_ERROR("Sever init failed!");
	SqlConnPool::instance()->close_pool();//关闭数据库

	return 0;
}

