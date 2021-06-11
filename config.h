// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once

//--------------------------Test-------------

  //#define TEST
  //#define PTHREAD
  //#define KA
  //#define _LOG_
  //#define _EPOLL_
  //#define _MYSQL_
  #define _WEB_

//----------------- MyEpoll---------------
#include <string>
constexpr int LISTEN_MAX = 1024;
/*
在Linux平台上，无论编写客户端程序还是服务端程序，
在进行高并发TCP连接处理时，最高的并发数量都要受到系统对用户单一进程
同时可打开文件数量的限制(这是因为系统为每个TCP连接都要创建一个socket句柄，
每个socket句柄同时也是一个文件句柄)。
可使用ulimit命令查看系统允许当前用户进程打开的文件数限制：
ulimit -n
*/

constexpr int MAX_EVENATS = 5000;//epoll_events结构体数组的大小


//----------------- TimerManager---------------

constexpr size_t TIME_OUT = 500;//ms.


constexpr int MAX_THREADS = 10240;
constexpr int MAX_QUEUE = 65535;

typedef enum {
	IMMEDIATE_SHUTDOWN = 1,
	WAIT_SHUTDOWN = 2
} threadpoll_shutdown_t;


//---------------------Task---------------------
constexpr int AGAIN_MAX_TIMES = 200;
// 有请求出现但是读不到数据,可能是Request Aborted,
// 或者来自网络的数据没有达到等原因,
// 对这样的请求尝试超过一定的次数就抛弃
//用于标识state的状态
constexpr int PARSE_URI = 1;
constexpr int PARSE_HEADERS = 2;
constexpr int RECV_BODY = 3;
constexpr int ANALYSIS = 4;
constexpr int FINISH = 5;

constexpr int BUFF_SIZE = 4096;

//用于标识PARSE_URI和PARSE_HEADERS和ANALYSIS时的信息
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





