// @Author Lin Tao
// @Email putaopu@qq.com
/*
2021.3.8
	Fightting!
*/

#include <iostream>
#include <unistd.h> //getopt�������������в���

#include "config.h"
#include "MyEpoll.h"
#include "Task.h"
#include "ThreadPool.h"
#include "TimerManager.h"

using namespace std;
__attribute((constructor)) void before(){
	printf("please enter the port and pthread num\n");
}

int main(int argc, char* argv[]) {
	//��������Ĳ����� -p port, 
	if (argc == 1) {
		cout << "the first arg is port_num, the second is pthread num,please divided by black:" << endl;
		return -1;
	}
	if (argc < 3) {
		cout << "Please enter the port num and phread num" << endl;
		return -1;
	}

	//�½�һ���ڴ�ض�����Ϊ�����̣߳�������������������ϵ�����
	shared_ptr<ThreadPool>threadpool(new ThreadPool(atoi(argv[2]), QUEUE_NUM));

	if (!threadpool->is_valid()) {
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
	shared_ptr<MyEpoll> myEpoll(new MyEpoll(atoi(argv[1]),threadpool,timerManager));
	if (!myEpoll->is_valid()) {
		cout << "myEpoll init failed!!!" << endl;
		return -1;
	}


	//����
	while(true){
		myEpoll->wait(MAX_EVENATS,TIME_OUT,PATH);
		timerManager->pop();//����ʱ�¼�,����������ɰ����ȶ��з�װ��֮��,��д��pop
	}

	return 0;
}
