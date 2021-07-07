/*2021.3.7Ҫ��ʼ��һ�γ����ˣ�ϣ��һ��˳��!*/

//--------------------
//���ر�Ҫͷ�ļ�
#include"util.h"
#include "epoll.h"
#include "requestData.h"
#include "threadpool.h"

#include <stdio.h>
#include <netinet/in.h> //��������ַ��
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


//����һЩ�����õ��ĳ���

const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;
const int LEN_LISTEN_QUE = 1024;

const int PORT = 8888;
const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

const string PATH = "/";

const int TIMER_TIME_OUT = 500;



//ʹ��һ��������requestData�����ʱ���,������
extern pthread_mutex_t qlock;
extern struct epoll_event* events;
extern priority_queue<MyTimer*, deque<MyTimer*>, timer_cmp> myTimerQueue;

void acceptConnection(int listen_fd, int epoll_fd, const string& path);

//socket_binf_listen: �󶨲�����һ��socket����,�����ظü���������
int socket_bind_listen(int port)
{
	//0-1023������ϵͳ�ģ����ܴ���65535
	if (port < 1024 || port > 65535)
	{
		printf("port_number error!!");
		return -1;
	}

	//����IPv4,tcp socket,���ؼ����ļ���

	//�ٴ���
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("create socket failed!!");
		return -1;
	}

	//������socket�����⵱ǰ�˿ںű�ռ�ö�����
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		printf("setsockopt failed!!");
		return -1;
	}

	//�����÷����� ip�Ͷ˿ںţ����Ұ󶨼���������
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//htonl��������ת�����޷��ų����͵������ֽ�˳��htons�Ƕ�����
	//INADDR_ANY:0.0.0.0;�����������ַ
	server_addr.sin_port = htons(static_cast<unsigned short>(port));

	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("bind failed!!");
		return -1;
	}

	//�ܰ���ɣ���ʼ����
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

//���ڽ���һ������
void accept_connection(int listen_fd, int epoll_fd, const string& path)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = 0;
	int accept_fd = 0;
	while ((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
	{
		//��������tcp����,�Ѹ��������óɷ������ģ�������ǵ����Ӷ��Ƿ�������
		int ret = set_sock_non_block(accept_fd);
		if (ret == -1)
		{
			perror("Set non block failed!");
			return;
		}

		//�½�һ�������������
		requestData* req_info = new requestData(epoll_fd, accept_fd, path);

		//io���ã�ʹ�����ܹ�ͬʱ��������ļ�������
		//ָ��һЩҪ������epoll�¼�
		//EPOLLIN:���ݿɶ�(������ͨ���ݺ���������)
		//EPOLLET:����ETģʽ���������ļ�������(���ش���)
		/*
		* epoll�����ִ����ķ�ʽ��LT��ˮƽ��������ET����Ե���������֣���ǰ�ߣ�ֻҪ�������¼��ͻ᲻�ϵĴ�����ֱ��������ɣ�
		������ֻ����һ����ͬ�¼�����˵ֻ�ڴӷǴ�������������״̬ת����ʱ����Ŵ�����
		����������һ�����������Ƕ��߳��ڴ���һ��SOCKET�¼����������ݿ�ʼ��������ʱ�����SOCKET������ͬ��һ���������¼���
		��������ݽ�����δ��ɣ���ô������Զ���������һ���̻߳��߽����������µ��¼��������һ�������ص����⣬��ͬ���̻߳��߽����ڴ���ͬһ��SOCKET���¼���
		���ʹ����Ľ�׳�Դ󽵵Ͷ���̵ĸ��Ӷȴ�����ӣ�����ʹ��ETģʽ��Ҳ�п��ܳ��������������
		����������������ַ�����һ�����ڵ������̻߳������������ݣ�Ҳ����˵���������ݵ��߳̽��յ����ݺ����̽�����ת����������̡߳�
		�ڶ��ַ�������EPOLLONESHOT���ַ�����������epoll��ע������¼���ע������¼���
		����ڴ���д�ɵ�ǰ��SOCKET��������ע������¼�����ô����¼��Ͳ�����Ӧ�˻���˵�����ˡ�
		Ҫ������ע���¼�����Ҫ����epoll_ctl�����ļ��������ϵ��¼���
		����ǰ���socket�Ͳ�����־�̬�����Ϳ���ͨ���ֶ��ķ�ʽ����֤ͬһSOCKETֻ�ܱ�һ���̴߳���
		�������Խ����̡߳�

		*/
		__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;

		//�����Լ���װ��epoll_add API
		epoll_add(epoll_fd, accept_fd, static_cast<void*>(req_info), _epo_event);
		//�����req_info��һ�¼��������ڱ�������epoll_event���涨���data_ptr��

		//����ʱ����Ϣ������Ӧ���õ���ʱ����/ʱ��ѣ�����Ĳ�����requestData����
		MyTimer* mtimer = new MyTimer(req_info, TIMER_TIME_OUT);
		req_info->addTimer(mtimer);
		pthread_mutex_lock(&qlock);
		myTimerQueue.push(mtimer);
		//myTimierQueue��һ����requestData�϶�������ȶ��У������õ���ʱ���
		//ʱ����ʹ�õ��ǹ̶�Ƶ�ʵ��Ĳ�����������ÿ���Ĳ���һ������Ҫ���������
		//ʱ������ǽ����ж�ʱ���г���ʱ����С��һ����ʱ���ĳ�ʱֵ��Ϊʱ����������ÿ���Ĳ�����������
		//��ʱʱ����С�Ķ�ʱ����Ȼ����
		pthread_mutex_unlock(&qlock);
	}
}


//�ַ������¼�
void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string& path, threadpool_t* tp)
{
	for (int i = 0; i < events_num; ++i)
	{
		//��ȡ�¼�������������
		//����ȡ��������requestData�е��¼�ָ�룬��ȡ��������
		requestData* request = (requestData*)(events[i].data.ptr);
		int fd = request->getFd();

		//��������������ӣ���������
		if (fd == listen_fd)
		{
			//���ո����ӣ���ʱҪ������Ž�ʱ�����һ���ȶ�����
			accept_connection(listen_fd, epoll_fd, path);
		}
		else
		{
			//�ų������¼�
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
				|| (!(events[i].events & EPOLLIN)))
				//���󣬹��𣬱���ܵ���д�˱��رպ󣬶������������յ����¼������ݲ��ɶ�
			{
				printf("error event\n!");
				delete request;
				continue;
			}


			//��ʱ����з�������¼���Ӧ��ʱ��ṹ�壬Ȼ��ִ������¼�������
			request->seperateTimer();
			//�¼��Ǵ���MyTimer����ṹ������
			//ִ�е������Ӧ�Ĵ�������myHandler��
			int rc = threadpool_add(tp, myHandler, events[i].data.ptr, 0);
		}
	}


	/* �����߼���������~
��Ϊ(1) ���ȶ��в�֧���������
(2) ��ʹ֧�֣����ɾ��ĳ�ڵ���ƻ��˶ѵĽṹ����Ҫ���¸��¶ѽṹ��
���Զ��ڱ���Ϊdeleted��ʱ��ڵ㣬���ӳٵ���(1)��ʱ �� (2)��ǰ��Ľڵ㶼��ɾ��ʱ�����Żᱻɾ����
һ���㱻��Ϊdeleted,����ٻ���TIMER_TIME_OUTʱ���ɾ����
�������������ô���
(1) ��һ���ô��ǲ���Ҫ�������ȶ��У�ʡʱ��
(2) �ڶ����ô��Ǹ���ʱʱ��һ�����̵�ʱ�䣬�����趨�ĳ�ʱʱ����ɾ��������(������һ����ʱʱ�������ɾ��)����������������ڳ�ʱ�����һ����������һ�γ����ˣ�
�Ͳ�������������requestData�ڵ��ˣ��������Լ����ظ�����ǰ���requestData��������һ��delete��һ��new��ʱ�䡣
*/

}

//����ʱ�¼�
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

	//��ʼ��epoll����
	int epoll_fd = epoll_init();
	//���ܣ�ע��epollʱ��������epoll_fd,��ʱ������const ��������������г���LISTENQ = 1024;
	//������һ���洢epoll_event �ṹ������飬�������С��MAX_EVENTS = 5000;
	if (epoll_fd < 0)
	{
		perror("epoll_init failed!!");
		return 1;
	}

	//��ʼ���̳߳�
	threadpool_t* threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);//4,65535��

	//�󶨼����˿�
	int listen_fd = socket_bind_listen(PORT);
	if (listen_fd < 0)
	{
		perror("socket bind failed!!");
		return 1;
	}

	//�������˿ڵĴ��䷽ʽ����Ϊ������
	if (set_sock_non_block(listen_fd) < 0)
	{
		perror("set socket non block failed!!");
		return 1;
	}
	
	//ע��ɱ��ش���
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
		epoll_wait����ֻ�ܻ�ȡ�Ƿ���ע���¼���������������¼�������ʲô��
		���ĸ�socket�������͵�ʱ�䡢���Ĵ�С�ȵ���Ϣ��ͳͳ��֪����
		*/
		//��ʱevents�Ѿ���Ϊ��һ���洢epoll_event���ݵĽṹ�壬�ýṹ��洢�����epoll�������������¼�
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

������
sock����Ҫ�����û��߻�ȡѡ����׽��֡�
level��ѡ�����ڵ�Э��㡣
optname����Ҫ���ʵ�ѡ������
optval������getsockopt()��ָ�򷵻�ѡ��ֵ�Ļ��塣����setsockopt()��ָ�������ѡ��ֵ�Ļ��塣
optlen������getsockopt()����Ϊ��ڲ���ʱ��ѡ��ֵ����󳤶ȡ���Ϊ���ڲ���ʱ��ѡ��ֵ��ʵ�ʳ��ȡ�����setsockopt()����ѡ��ĳ��ȡ�
*/
