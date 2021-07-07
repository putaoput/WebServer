#include "epoll.h"

#include <sys/epoll.h>
#include<errno.h>
#include"threadpool.h"
#include<stdio.h>

struct epoll_event* events;//定义一个指向epoll_event的指针
/*
struct epoll_event
{
  uint32_t events;    Epoll events 
  epoll_data_t data;    User data variable 
	}

*/

//初始化epoll
int epoll_init()
{
	int epoll_fd = epoll_create(LISTENQ + 1);
	//参数告诉内核需要的事件表的大小。返回的文件描述符将用作其他所有epoll系统调用的第一个参数。
	if (epoll_fd == -1)
	{
		printf("epoll_create failed!!");
		return 1;
	}

	events = new epoll_event[MAX_EVENTS];
	//所以这里events是一个epoll事件数组;
	return epoll_fd;

}

//注册新描述符
int epoll_add(int epoll_fd, int fd, void* request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	//这里events是所有监听event的或

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		/*
		EPOLL_CTL_ADD，往事件表上注册fd上的事件
		EPOLL_CTL_MOD，修改fd上的注册事件
		EPOLL_CTl_DEL，删除fd上的注册事件
		*/
		perror("epoll_add error");
		return -1;
	}

	return 0;
}

//修改事件描述符的状态
int epoll_mod(int epoll_fd, int fd, void* request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		perror("epoll_mod error!!\n");
		/*
		perror(s) 用来将上一个函数发生错误的原因输出到标准设备(stderr)。
		参数 s 所指的字符串会先打印出，后面再加上错误原因字符串。
		此错误原因依照全局变量errno的值来决定要输出的字符串。
		*/
		return -1;
	}
	return 0;
}

//从epoll_fd事件组中删除描述符
int epoll_del(int epoll_fd, int fd, void* request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		perror("epoll_del error!!\n");
		return -1;
	}
	return 0;
}

//返归活跃事件数
int my_epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout)
{
	int ret_count = epoll_wait(epoll_fd, events, max_events, timeout);

	/*
	epoll_wait 在一段超时时间内等待一组文件描述符上的事件，成功时返回就绪的文件描述符的个数;
	*/
	if (ret_count < 0)
	{
		perror("epoll wait error!!");
	}

	return ret_count;
}
