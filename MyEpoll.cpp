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
using namespace std;

MyEpoll::MyEpoll(int _port, std::shared_ptr<TimerManager> _timerManager):
	isValid(false),
	timerManager(_timerManager)
{
	eventsArr = new epoll_event[MAX_EVENATS];
	if (start(_port) == -1 || epoll_fd < 0) {
		return;
	}
	//��һ�����ļ�������ע�ᵽepoll�¼���
	
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
			#ifdef TEST
				cout << "After add!!!" << endl;
				cout << "This is listen_fd = " << event.data.fd << endl;
				cout << "This is event = " << event.events << endl;
			#endif
	
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		perror("epoll add failed!!\n");
		return -1;
	}
	epollTask[_task->get_fd()] = _task;

	return 0;
}

int MyEpoll::mod(shared_ptr<Task> _task) {
	struct epoll_event event;
	event.data.fd = _task->get_fd();
	event.events = _task->get_events();
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, _task->get_fd(), &event) < 0)
	{
		perror("epoll_mod failed!!!\n");
		return -1;
	}
	epollTask[_task->get_fd()] = _task;
	return 0;
}

int MyEpoll::del(shared_ptr<Task> _task) {
	struct epoll_event event;
	event.data.fd = _task->get_fd();
	event.events = _task->get_events();
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _task->get_fd(), &event) < 0)
	{
		perror("epoll_del failed!!!\n");
		return -1;
	}
	epollTask[_task->get_fd()] = _task;
	return 0;
}

int MyEpoll::wait(int maxEvents, int timeOut, string _path) {
	int eventCount = epoll_wait(epoll_fd, eventsArr, maxEvents, timeOut);
	//events ��洢�˼������������¼���
	if (eventCount < 0) {
		perror("epoll wait error");
		return -1;
	}
	//����Щ�¼����ദ����װ���͸��̳߳�
	for (int i = 0; i < eventCount; ++i)
	{
		#ifdef TEST
			cout << "eventCount = " << eventCount << endl;
		#endif
		// ��ȡ���¼�������������
		int fd = eventsArr[i].data.fd;

		// ���¼�������������Ϊ����������
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
			// �ų������¼�
			if ((eventsArr[i].events & EPOLLERR) || (eventsArr[i].events & EPOLLHUP)
				|| (!(eventsArr[i].events & EPOLLIN)))
			{
				//printf("error event\n");
				auto fdIter = epollTask.find(fd);
				if (fdIter != epollTask.end())
					epollTask.erase(fdIter);
				//printf("fd = %d, here\n", fd);
				continue;
			}

			// ������������뵽�̳߳���
			// �����̳߳�֮ǰ��Timer��request����
			auto task = epollTask[fd];
			
			//printf("cur_req.=%d\n", cur_req.use_count());
			
			ThreadPool::add(shared_ptr<ThreadPoolTask>(new ThreadPoolTask(task)));//�����̳߳�֮��Ҫ��װһ��Task;
			
			
		}
	}
	
	return -1;
}

int MyEpoll::start(int _port) {

	if (_port < 1024 || _port > 65535) {
		cout << "input error port number!!" << endl;
		return -1;
	}

	
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("listen_fd create failed!!!\n");
		return -1;
	}

	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror("setsockopt failed!!!\n");
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)_port);

	if ((bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
		perror("bind failed!!!\n");
		return -1;
	}

	if (listen(listen_fd, LISTEN_MAX) == -1 || listen_fd == -1) {
		perror("listen failed!!!");
		close(listen_fd);
		return -1;
	}

	//�Ѿ�������ɣ���һ����Ϊ��������Ȼ�󴴽�epoll_fd������
	if (set_sock_non_block(listen_fd) < 0)
	{
		perror("set socket non block failed");
		return -1;
	}

	if ((epoll_fd = epoll_create(LISTEN_MAX + 1)) == -1) {
		perror("create epoll_fd failed!!");
		return -1;
	}

	//����listen_fdע�ᵽepoll_fd��
	__uint32_t events = EPOLLIN | EPOLLET;
	shared_ptr<Task> task( new Task(listen_fd, shared_ptr<MyEpoll>(this), TIME_OUT, PATH, timerManager, events));
	
//ע�����task�������ջ�Ϲ��죬û��ʹ��boost::shared_ptr<D>����boost::shared_ptr<D>�е�weak����ptr��ָ����Ҳ��û�б���ֵ���ᵼ��bad_weak_ptr,
//����һ�־����ڹ��캯����ʹ��shared_from_this

	if ((add(task)) == -1) {
		return -1;
	}
	
	return 0;
}

int MyEpoll::set_sock_non_block(int _listen_fd) {
	int flag = fcntl(_listen_fd, F_GETFL, 0);
	if (flag == -1) {
		perror("F_GETEL failed!!\n");
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(_listen_fd, F_SETFL, flag) == -1) {
		perror("F_SETFL failed!!\n");
		return -1;
	}
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
			return -1;
		}

		//�½�һ������
		// �ļ����������Զ�����Ե����(Edge Triggered)ģʽ����֤һ��socket��������һʱ��ֻ��һ���̴߳���
		__uint32_t event = EPOLLIN | EPOLLET | EPOLLONESHOT;
			#ifdef TEST
				cout << "Before add" << endl;
				cout << "This is listen_fd = " << _fd << endl;
				cout << "This is event = " << event << endl;
			#endif
		shared_ptr<Task> task(new Task(acceptFd, shared_ptr<MyEpoll>(this), _timeOut, _path, timerManager, event));
		add(task);
		#ifdef TEST
		cout << "sp_task -> get_fd()" << task->get_fd() << endl;
		#endif
		task->push_to();
	}

	return 0;
}


