// @Author Lin Tao
// @Email putaopu@qq.com

#include <signal.h>
#include <iostream>
#include <string.h>
//#include <unistd.h> //getopt�������������в���

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
//__attribute((constructor)) void before(){
//	printf("please enter the port and pthread num\n");
//}

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

	//�����½���־�������¼���������������
	Log::instance()->init(0, "./Log", "Log", 1024);
	//���ӵ����ݿ�
	//SqlConnPool::instance()->init(SERVER_HOST, SERVER_SQLPORT, SERVER_SQLUSER, SERVER_SQLPWD, SERVER_DBNAME, SERVER_SQLCOONPOOL_NUM);
	SqlConnPool::instance()->init("localhost", 3306, "lintao", "123456", "myServer",1024);

	//��ʼ��һЩ������
	MimeType::instance()->init();
	WebPage::instance()->init();
#ifdef  _LOG_
		LOG_INFO("Test LOG_INFO");
		LOG_ERROR("Test LOG_ERROR");
		LOG_DEBUG("Test LOG_DEBUG");
		LOG_WARN("Test LOG_WARN");
#endif //  _LOG_
			
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
		ThreadPool::create(atoi(argv[2]), QUEUE_NUM);
		if (!ThreadPool::is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}

		//�½�һ����ʱ�����������г�ʱ����
		shared_ptr<TimerManager> timerManager(new TimerManager);
		if (!timerManager->is_valid()) {
			cout << "timerManager init failed!!!\n" << endl;
			return -1;
		}

		//�½�һ��epoll������Ϊreactorģ�͵����̣߳��������еĶ�д�����¼�
		shared_ptr<MyEpoll> myEpoll(new MyEpoll(atoi(argv[1]), timerManager));
		if (!myEpoll->is_valid()) {
			cout << "myEpoll init failed!!!" << endl;
			return -1;
		}
	
		//Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
		LOG_INFO("The WebServer init");
		LOG_INFO("Port:%d", atoi(argv[1]),"           Thread num:%d",atoi(argv[2]));

	//����
		while (true) {
			myEpoll->wait(MAX_EVENATS, -1, PATH);
			timerManager->pop();//����ʱ�¼�,����������ɰ����ȶ��з�װ��֮��,��д��pop
		}
	} while (false)
	LOG_ERROR("Sever init failed!");
	SqlConnPool::instance()->close_pool();//�ر����ݿ�

	return 0;
}

/*
ͷ�ļ�
#include<mysql.h>
����ԭ������:
MYSQL *mysql_real_connect (MYSQL *mysql,
const char *host,
const char *user, 
const char *passwd, 
const char *db, 
unsigned int port,
const char *unix_socket,
unsigned long client_flag)
���������������������Ҫȡֵ��
MYSQL *Ϊmysql_init�������ص�ָ�룬
hostΪnull�� localhostʱ���ӵ��Ǳ��صļ������
��mysqlĬ�ϰ�װ��unix������unix��ϵͳ�У�root�˻���û������ģ�����û���ʹ��root������Ϊnull��
��dbΪ�յ�ʱ�򣬺������ӵ�Ĭ�����ݿ⣬�ڽ��� mysql��װʱ�����Ĭ�ϵ�test���ݿ⣬��˴˴�����ʹ��test���ݿ����ƣ�
port�˿�Ϊ0��
ʹ�� unix���ӷ�ʽ��unix_socketΪnullʱ��������ʹ��socket��ܵ����ƣ����һ��������������Ϊ0
mysql_real_connect()�����������������ϵ�MySQL���ݿ����潨�����ӡ������ܹ�ִ����Ҫ��ЧMySQL���Ӿ���ṹ���κ�����API����֮ǰ��mysql_real_connect()����ɹ���ɡ� 
*/