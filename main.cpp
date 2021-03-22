// @Author Lin Tao
// @Email putaopu@qq.com
/*
2021.3.8
	Fightting!
*/

#include <iostream>
#include <unistd.h> //getopt用来分析命令行参数

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
	//可以输出的参数有 -p port, 
	if (argc == 1) {
		cout << "the first arg is port_num, the second is pthread num,please divided by black:" << endl;
		return -1;
	}
	if (argc < 3) {
		cout << "Please enter the port num and phread num" << endl;
		return -1;
	}

	//新建一个内存池对象，作为工作线程，处理所有在请求队列上的任务
	shared_ptr<ThreadPool>threadpool(new ThreadPool(atoi(argv[2]), QUEUE_NUM));

	if (!threadpool->is_valid()) {
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
	shared_ptr<MyEpoll> myEpoll(new MyEpoll(atoi(argv[1]),threadpool,timerManager));
	if (!myEpoll->is_valid()) {
		cout << "myEpoll init failed!!!" << endl;
		return -1;
	}


	//启动
	while(true){
		myEpoll->wait(MAX_EVENATS,TIME_OUT,PATH);
		timerManager->pop();//处理超时事件,这里可以理解成把优先队列封装完之后,重写了pop
	}

	return 0;
}
