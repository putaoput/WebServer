// @Author Lin Tao
// @Email putaopu@qq.com

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>	//inet_addr
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string.h>//bzero
#include <unistd.h> //close

#include "MyEpoll.h"
#include "config.h"
#include "Task.h"
#include "TimerManager.h"
#include "SingerTimer.h"
#include "Log.h"
using namespace std;

class helper{
public:
	helper(std::shared_ptr<Task> _task){
		 task = _task;
	}
	std::shared_ptr<Task> task;
};

MyEpoll::MyEpoll(int _port, std::shared_ptr<TimerManager> _timerManager):
	isValid(false),
	timerManager(_timerManager)
{
	eventsArr = new epoll_event[MAX_EVENATS];
	if (start(_port) == -1 || epoll_fd < 0) {
		return;
	}
	//下一步将文件描述符注册到epoll事件表	
	isValid = true;
}

bool MyEpoll::is_valid() {
	return  isValid;
}

int MyEpoll::add(shared_ptr<Task> _task) {
	
	struct  epoll_event event;
	event.data.fd = _task->get_fd();
	event.events = _task->get_events();
	int fd = _task->get_fd();			
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		perror("epoll add failed!!\n");
		LOG_ERROR("ADD fd:%d to epoll_fd failed!!", _task->get_fd());
		return -1;
	}
	epollTask[_task->get_fd()] = _task;
	LOG_INFO("Add a new fd:%d to epoll_fd", _task->get_fd());
	return 0;
}

int MyEpoll::mod(shared_ptr<Task> _task) {
	struct epoll_event event;
	event.data.fd = _task->get_fd();
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, _task->get_fd(), &event) < 0)
	{
		LOG_ERROR("Mod fd:%d  failed!!", _task->get_fd());
		perror("epoll_mod failed!!!\n");
		return -1;
	}
	epollTask[_task->get_fd()] = _task;
	return 0;
	LOG_INFO("Mod fd:%d  failed!!", _task->get_fd());
}

int MyEpoll::del(shared_ptr<Task> _task) {
	if(epollTask.find(_task->get_fd()) == epollTask.end()){
		cout << "not del" << endl;
		return 0;
		//这是因为如果是短连接，那么需要提前执行del，不然无法正常释放资源。
	}
	struct epoll_event event;
	event.data.fd = _task->get_fd();
	event.events = _task->get_events();
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _task->get_fd(), &event) < 0)
	{
		perror("epoll_del failed!!!\n");
		LOG_ERROR("Del fd:%d  failed!!", _task->get_fd());
		return -1;
	}
	
	if(epollTask.find(_task->get_fd()) != epollTask.end()){
		new helper(_task);
		epollTask.erase(_task->get_fd());
		close(_task->get_fd());
		LOG_INFO("Del fd:%d !!", _task->get_fd());
	}

	return 0;
}

int MyEpoll::del(int _fd){
	struct epoll_event event;
	event.data.fd = _fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _fd, &event) < 0)
	{
		perror("epoll_del failed!!!\n");
		LOG_ERROR("Del fd:%d  failed!!", _fd);
		return -1;
	}
	if(epollTask.find(_fd )!= epollTask.end()){
		new helper (epollTask[_fd]);
		epollTask.erase(_fd);
		close(_fd);
		LOG_INFO("Del fd:%d!!", _fd);
	}
	return 0;
}

int MyEpoll::wait(int maxEvents, int _timeOut, string _path) {
	int eventCount = epoll_wait(epoll_fd, eventsArr, maxEvents, _timeOut);
	//timeOut指定epoll的超时值，如果为-1将永远阻塞
	//events 里存储了监听到的所有事件，
	if (eventCount < 0) {
		perror("epoll wait error");
		LOG_ERROR("epoll wait error");
		return -1;
}
		LOG_INFO("eventCount = %d", eventCount);
	//将这些事件分类处理，包装好送给线程池
	for (int i = 0; i < eventCount; ++i)
	{
		// 获取有事件产生的描述符
		int fd = eventsArr[i].data.fd;
		// 有事件发生的描述符为监听描述符
		if (fd == listen_fd)
		{			
			accept_connection(listen_fd, epoll_fd, _path);
		}
		else if (fd < 3)
		{
			return -1;
		}
		else
		{
			// 排除错误事件
			if ((eventsArr[i].events & EPOLLERR) || (eventsArr[i].events & EPOLLHUP)
				|| (!(eventsArr[i].events & EPOLLIN)))
			{
				//printf("error event\n");
				auto fdIter = epollTask.find(fd);
				if (fdIter != epollTask.end())
					epollTask.erase(fdIter);
				//printf("fd = %d, here\n", fd);

				LOG_ERROR("Unexpected event");
				continue;
			}

			// 将请求任务加入到线程池中
			// 加入线程池之前将Timer和request分离
			shared_ptr<Task> task = epollTask[fd];
			//并且从任务池里面删除该task，保证可以即使析构无效的连接,如果时长连接，需要重新加入
			//epollTask.erase(fd);
			task->separate();
			
			ThreadPool::add(shared_ptr<ThreadPoolTask>(new ThreadPoolTask(task)));//加入线程池之后，要封装一次Task;	
		}
	}
	
	return -1;
}

int MyEpoll::start(int _port) {

	if (_port < 1024 || _port > 65535) {
		cout << "input error port number!!" << endl;
		LOG_ERROR("Port:%d error!", _port);
		return -1;
	}

	
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("listen_fd create failed!!!\n");
		LOG_ERROR("Create socket of port:%d error!", _port);
		return -1;
	}

	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror("setsockopt failed!!!\n");
		LOG_ERROR("port:%d setsockopt  socket error!", _port);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)_port);

	if ((bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
		perror("bind failed!!!\n");
		LOG_ERROR("port:%d bind error!", _port);
		return -1;
	}

	if (listen(listen_fd, LISTEN_MAX) == -1 || listen_fd == -1) {
		perror("listen failed!!!");
		LOG_ERROR("bind of port:%d error!", _port);
		close(listen_fd);
		return -1;
	}

	//已经监听完成，下一步设为非阻塞，然后创建epoll_fd描述符
	if (set_sock_non_block(listen_fd) < 0)
	{	
		LOG_ERROR("set socket non block of fd:%d failed", listen_fd);
		perror("set socket non block failed");
		return -1;
	}

	if ((epoll_fd = epoll_create(LISTEN_MAX + 1)) == -1) {
		LOG_ERROR("create epoll_fd failed!!");
		perror("create epoll_fd failed!!");
		return -1;
	}

	//将该listen_fd注册到epoll_fd上
	__uint32_t events = EPOLLIN | EPOLLET;
	shared_ptr<Task> task( new Task(listen_fd, shared_ptr<MyEpoll>(this), TIME_OUT, PATH, timerManager, events));
	
//注意这个task如果是在栈上构造，没有使用boost::shared_ptr<D>所以boost::shared_ptr<D>中的weak——ptr所指对象也就没有被赋值，会导致bad_weak_ptr,
//还有一种就是在构造函数中使用shared_from_this

	if ((add(task)) == -1) {
		return -1;
	}
	
	return 0;
}

int MyEpoll::set_sock_non_block(int _listen_fd) {
	int flag = fcntl(_listen_fd, F_GETFL, 0);
	if (flag == -1) {
		perror("F_GETEL failed!!\n");
		LOG_ERROR("Set fd%d non_block,failed", _listen_fd);
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(_listen_fd, F_SETFL, flag) == -1) {
		perror("F_SETFL failed!!\n");
		LOG_ERROR("Set fd%d non_block,failed", _listen_fd);
		return -1;
	}

	LOG_INFO("Set fd%d non_block", _listen_fd);
	return 0;
}

int MyEpoll::accept_connection(int _fd, size_t _timeOut, string _path) {
	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(struct sockaddr_in));
	socklen_t clientAddrLen = 0;
	int acceptFd = 0;
	while ((acceptFd = accept(_fd, (struct sockaddr*)&clientAddr, &clientAddrLen)) > 0)
	{
		if (set_sock_non_block(acceptFd) < 0) {
			perror("Set non block failed!!!\n");
			LOG_WARN("Set nonblock error of client:%d", _fd);
			return -1;
		}

		//新建一个任务
		// 文件描述符可以读，边缘触发(Edge Triggered)模式，保证一个socket连接在任一时刻只被一个线程处理
		__uint32_t event = EPOLLIN | EPOLLET | EPOLLONESHOT;
		shared_ptr<Task> task(new Task(acceptFd, shared_ptr<MyEpoll>(this), _timeOut, _path, timerManager, event));
		add(task);
		//新建一个计时器，负责绑定该task;
		shared_ptr<SingleTimer> singleTimer(new SingleTimer(TIME_OUT));
		task->set_singleManager(singleTimer);
		timerManager->add(singleTimer);
		
	}

	LOG_INFO("Accept a new connetion!  the fd of client is %d", _fd);
	return 0;
}


