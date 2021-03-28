// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once

//--------------------------Test-------------
//#define TEST
//#define PTHREAD
#define KA

//----------------- MyEpoll---------------
#include <string>
constexpr int LISTEN_MAX = 1024;
/*
��Linuxƽ̨�ϣ����۱�д�ͻ��˳����Ƿ���˳���
�ڽ��и߲���TCP���Ӵ���ʱ����ߵĲ���������Ҫ�ܵ�ϵͳ���û���һ����
ͬʱ�ɴ��ļ�����������(������ΪϵͳΪÿ��TCP���Ӷ�Ҫ����һ��socket�����
ÿ��socket���ͬʱҲ��һ���ļ����)��
��ʹ��ulimit����鿴ϵͳ����ǰ�û����̴򿪵��ļ������ƣ�
[speng@as4 ~]$ ulimit -n


*/

constexpr int MAX_EVENATS = 5000;//epoll_events�ṹ������Ĵ�С


//----------------- TimerManager---------------

constexpr size_t TIME_OUT = 500;//ms.


constexpr int MAX_THREADS = 1024;
constexpr int MAX_QUEUE = 65535;

typedef enum {
	IMMEDIATE_SHUTDOWN = 1,
	WAIT_SHUTDOWN = 2
} threadpoll_shutdown_t;


//---------------------Task---------------------
constexpr int AGAIN_MAX_TIMES = 200;
// ��������ֵ��Ƕ���������,������Request Aborted,
// �����������������û�дﵽ��ԭ��,
// �������������Գ���һ���Ĵ���������
//���ڱ�ʶstate��״̬
constexpr int PARSE_URI = 1;
constexpr int PARSE_HEADERS = 2;
constexpr int RECV_BODY = 3;
constexpr int ANALYSIS = 4;
constexpr int FINISH = 5;

constexpr int BUFF_SIZE = 4096;

//���ڱ�ʶPARSE_URI��PARSE_HEADERS��ANALYSISʱ����Ϣ
constexpr int STATE_AGAIN = -1;
constexpr int STATE_ERROR = -2;
constexpr int STATE_SUCCESS = 0;


constexpr int METHOD_POST = 1;
constexpr int METHOD_GET = 2;
constexpr int HTTP_10 = 1;
constexpr int HTTP_11 = 2;

constexpr int EPOLL_WAIT_TIME = 500;

enum HeadersState
{
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACE,
    H_VALUE,
    H_R,
    H_N,
    H_FINAL_R,
    H_FINAL_N
};

//--------------------main---------------------
constexpr int QUEUE_NUM = 65535;
const std::string PATH = "/";


