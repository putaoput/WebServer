/*2021.3.7要开始第一次尝试了，希望一切顺利!*/

//--------------------
//加载必要头文件
#include"util.h"
#include "epoll.h"
#include "requestData.h"
#include "threadpool.h"

#include <stdio.h>
#include <netinet/in.h> //互联网地址簇
#include <arpa/inet.h> //
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include <deque>
#include<queue>
#include<vector>

//test
#include <iostream>

using namespace std;


//定义一些将会用到的常量

const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;
const int LEN_LISTEN_QUE = 1024;

const int PORT = 8888;
const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

const string PATH = "/";

const int TIMER_TIME_OUT = 500;



//使用一个定义在requestData里面的时间堆,互斥锁
extern pthread_mutex_t qlock;
extern struct epoll_event* events;
extern priority_queue<MyTimer*, deque<MyTimer*>, timer_cmp> myTimerQueue;

void acceptConnection(int listen_fd, int epoll_fd, const string& path);

//socket_binf_listen: 绑定并监听一个socket连接,并返回该监听的描述
int socket_bind_listen(int port)
{
	//0-1023是留给系统的，不能大于65535
	if (port < 1024 || port > 65535)
	{
		printf("port_number error!!");
		return -1;
	}

	//创建IPv4,tcp socket,返回监听文件符

	//①创建
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("create socket failed!!");
		return -1;
	}

	//②设置socket，避免当前端口号被占用而报错
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		printf("setsockopt failed!!");
		return -1;
	}

	//③设置服务器 ip和端口号，并且绑定监听描述符
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//htonl将主机数转换成无符号长整型的网络字节顺序，htons是短整型
	//INADDR_ANY:0.0.0.0;本机的任意地址
	server_addr.sin_port = htons(static_cast<unsigned short>(port));

	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("bind failed!!");
		return -1;
	}

	//④绑定完成，开始监听
	if (listen(listen_fd, LEN_LISTEN_QUE) == -1)
	{
		printf("listen failed!!");
		return -1;
	}

	if (listen_fd == -1)
	{
		close(listen_fd);
		printf("invalid linsten_fd!!");
		return -1;
	}

	return listen_fd;
}


void myHandler(void* args)
{
	requestData* req_data = (requestData*) args;
	req_data->handleRequest();
}

//用于接收一个连接
void accept_connection(int listen_fd, int epoll_fd, const string& path)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = 0;
	int accept_fd = 0;
	while ((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
	{
		//监听到了tcp连接,把该连接设置成非阻塞的，因此我们的连接都是非阻塞的
		int ret = set_sock_non_block(accept_fd);
		if (ret == -1)
		{
			perror("Set non block failed!");
			return;
		}

		//新建一个数据请求对象
		requestData* req_info = new requestData(epoll_fd, accept_fd, path);

		//io复用，使程序能够同时监听多个文件描述符
		//指定一些要监听的epoll事件
		//EPOLLIN:数据可读(包括普通数据和优先数据)
		//EPOLLET:将以ET模式来操作该文件描述符(边沿触发)
		/*
		* epoll有两种触发的方式即LT（水平触发）和ET（边缘触发）两种，在前者，只要存在着事件就会不断的触发，直到处理完成，
		而后者只触发一次相同事件或者说只在从非触发到触发两个状态转换的时候儿才触发。
		这会出现下面一种情况，如果是多线程在处理，一个SOCKET事件到来，数据开始解析，这时候这个SOCKET又来了同样一个这样的事件，
		而你的数据解析尚未完成，那么程序会自动调度另外一个线程或者进程来处理新的事件，这造成一个很严重的问题，不同的线程或者进程在处理同一个SOCKET的事件，
		这会使程序的健壮性大降低而编程的复杂度大大增加！！即使在ET模式下也有可能出现这种情况！！
		解决这种现象有两种方法，一种是在单独的线程或进程里解析数据，也就是说，接收数据的线程接收到数据后立刻将数据转移至另外的线程。
		第二种方法就是EPOLLONESHOT这种方法，可以在epoll上注册这个事件，注册这个事件后，
		如果在处理写成当前的SOCKET后不再重新注册相关事件，那么这个事件就不再响应了或者说触发了。
		要想重新注册事件则需要调用epoll_ctl重置文件描述符上的事件，
		这样前面的socket就不会出现竞态这样就可以通过手动的方式来保证同一SOCKET只能被一个线程处理
		，不会跨越多个线程。

		*/
		__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;

		//调用自己封装的epoll_add API
		epoll_add(epoll_fd, accept_fd, static_cast<void*>(req_info), _epo_event);
		//在这里，req_info这一事件请求是在被放在了epoll_event里面定义的data_ptr里

		//新增时间信息，这里应该用到了时间轮/时间堆，具体的操作在requestData里面
		MyTimer* mtimer = new MyTimer(req_info, TIMER_TIME_OUT);
		req_info->addTimer(mtimer);
		pthread_mutex_lock(&qlock);
		myTimerQueue.push(mtimer);
		//myTimierQueue是一个在requestData上定义的优先队列，这里用的是时间堆
		//时间轮使用的是固定频率的心博函数，所以每次心博不一定有需要处理的任务
		//时间堆则是将所有定时器中超过时间最小的一个定时器的超时值作为时间间隔，这样每次心博函数被调用
		//超时时间最小的定时器必然到期
		pthread_mutex_unlock(&qlock);
	}
}


//分发处理事件
void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string& path, threadpool_t* tp)
{
	for (int i = 0; i < events_num; ++i)
	{
		//获取事件产生的描述符
		//首先取出定义在requestData中的事件指针，再取出描述符
		requestData* request = (requestData*)(events[i].data.ptr);
		int fd = request->getFd();

		//如果监听到新连接，则建立连接
		if (fd == listen_fd)
		{
			//接收该连接，此时要把请求放进时间堆这一优先队列里
			accept_connection(listen_fd, epoll_fd, path);
		}
		else
		{
			//排除错误事件
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
				|| (!(events[i].events & EPOLLIN)))
				//错误，挂起，比如管道的写端被关闭后，读端描述符将收到该事件，数据不可读
			{
				printf("error event\n!");
				delete request;
				continue;
			}


			//从时间堆中分离这个事件对应的时间结构体，然后执行这个事件的请求
			request->seperateTimer();
			//事件是存在MyTimer这个结构体里面
			//执行的请求对应的处理函数是myHandler。
			int rc = threadpool_add(tp, myHandler, events[i].data.ptr, 0);
		}
	}


	/* 处理逻辑是这样的~
因为(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请requestData节点了，这样可以继续重复利用前面的requestData，减少了一次delete和一次new的时间。
*/

}

//处理超时事件
void handle_expired_event()
{
	pthread_mutex_lock(&qlock);
	while (!myTimerQueue.empty())
	{
		MyTimer* ptimer_now = myTimerQueue.top();
		if (ptimer_now->isvalid() == false)
		{
			myTimerQueue.pop();
			delete ptimer_now;
		}
		else
		{
			break;
		}
	}
	pthread_mutex_unlock(&qlock);
}

int main()
{
	handle_for_sigpipe();

	//初始化epoll调用
	int epoll_fd = epoll_init();
	//功能：注册epoll时间描述符epoll_fd,此时传入了const 定义的最大监听队列长度LISTENQ = 1024;
	//定义了一个存储epoll_event 结构体的数组，该数组大小：MAX_EVENTS = 5000;
	if (epoll_fd < 0)
	{
		perror("epoll_init failed!!");
		return 1;
	}

	//初始化线程池
	threadpool_t* threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);//4,65535。

	//绑定监听端口
	int listen_fd = socket_bind_listen(PORT);
	if (listen_fd < 0)
	{
		perror("socket bind failed!!");
		return 1;
	}

	//将监听端口的传输方式设置为非阻塞
	if (set_sock_non_block(listen_fd) < 0)
	{
		perror("set socket non block failed!!");
		return 1;
	}
	
	//注册成边沿触发
	__uint32_t event = EPOLLIN | EPOLLET;
	requestData* req = new requestData();
	req->setFd(listen_fd);
	epoll_add(epoll_fd, listen_fd, static_cast<void*>(req), event);
	
	//test
	//cout << "Before while!" << endl;

	while (true)
	{
		int events_num = my_epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		/*
		epoll_wait函数只能获取是否有注册事件发生，至于这个事件到底是什么、
		从哪个socket来、发送的时间、包的大小等等信息，统统不知道。
		*/
		//此时events已经成为了一个存储epoll_event数据的结构体，该结构体存储了这次epoll监听到的所有事件
		if (events_num == 0)
			continue;
		printf("the nums of envents is %d\n", events_num);

		handle_events(epoll_fd, listen_fd, events, events_num, PATH, threadpool);
		

              //  cout << "before handle_expired_event" << endl;

		handle_expired_event();
	}

	return 0;

}
/*
int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

参数：
sock：将要被设置或者获取选项的套接字。
level：选项所在的协议层。
optname：需要访问的选项名。
optval：对于getsockopt()，指向返回选项值的缓冲。对于setsockopt()，指向包含新选项值的缓冲。
optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。对于setsockopt()，现选项的长度。
*/
