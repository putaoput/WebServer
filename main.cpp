// @Author Lin Tao
// @Email putaopu@qq.com

#include <signal.h>
#include <iostream>
#include <string.h>
//#include <unistd.h> //getopt�������������в���

#include "config.h"
#include "MyEpoll.h"
#include "Task.h"
#include "ThreadPool.h"
#include "TimerManager.h"
#include "Log.h"

using namespace std;
//__attribute((constructor)) void before(){
//	printf("please enter the port and pthread num\n");
//}

int main(int argc, char* argv[]) {

	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL)) {
		return -1;
	}

	//��������Ĳ����� -p port, 
	if (argc == 1) {
		cout << "the first arg is port_num, the second is pthread num,please divided by black:" << endl;
		return -1;
	}
	if (argc < 3) {
		cout << "Please enter the port num and phread num" << endl;
		return -1;
	}
	//������־�洢·��
	const string logFilePath = "./logFile.log";
	//�½�һ���ڴ�ض�����Ϊ�����̣߳�������������������ϵ�����

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

		LOG_INFO("Sever init");
		LOG_INFO("Port:%d", argv[1],"           Thread num:%d",argv[2]);

	//����
		while (true) {
			myEpoll->wait(MAX_EVENATS, -1, PATH);
			//timerManager->pop();//����ʱ�¼�,����������ɰ����ȶ��з�װ��֮��,��д��pop
		}
	} while (false);

	LOG_ERROR("Sever init failed!")
		
	return 0;
}
