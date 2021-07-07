#include "epoll.h"

#include <sys/epoll.h>
#include<errno.h>
#include"threadpool.h"
#include<stdio.h>

struct epoll_event* events;//����һ��ָ��epoll_event��ָ��
/*
struct epoll_event
{
  uint32_t events;    Epoll events 
  epoll_data_t data;    User data variable 
	}

*/

//��ʼ��epoll
int epoll_init()
{
	int epoll_fd = epoll_create(LISTENQ + 1);
	//���������ں���Ҫ���¼���Ĵ�С�����ص��ļ���������������������epollϵͳ���õĵ�һ��������
	if (epoll_fd == -1)
	{
		printf("epoll_create failed!!");
		return 1;
	}

	events = new epoll_event[MAX_EVENTS];
	//��������events��һ��epoll�¼�����;
	return epoll_fd;

}

//ע����������
int epoll_add(int epoll_fd, int fd, void* request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	//����events�����м���event�Ļ�

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		/*
		EPOLL_CTL_ADD�����¼�����ע��fd�ϵ��¼�
		EPOLL_CTL_MOD���޸�fd�ϵ�ע���¼�
		EPOLL_CTl_DEL��ɾ��fd�ϵ�ע���¼�
		*/
		perror("epoll_add error");
		return -1;
	}

	return 0;
}

//�޸��¼���������״̬
int epoll_mod(int epoll_fd, int fd, void* request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		perror("epoll_mod error!!\n");
		/*
		perror(s) ��������һ���������������ԭ���������׼�豸(stderr)��
		���� s ��ָ���ַ������ȴ�ӡ���������ټ��ϴ���ԭ���ַ�����
		�˴���ԭ������ȫ�ֱ���errno��ֵ������Ҫ������ַ�����
		*/
		return -1;
	}
	return 0;
}

//��epoll_fd�¼�����ɾ��������
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

//�����Ծ�¼���
int my_epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout)
{
	int ret_count = epoll_wait(epoll_fd, events, max_events, timeout);

	/*
	epoll_wait ��һ�γ�ʱʱ���ڵȴ�һ���ļ��������ϵ��¼����ɹ�ʱ���ؾ������ļ��������ĸ���;
	*/
	if (ret_count < 0)
	{
		perror("epoll wait error!!");
	}

	return ret_count;
}
