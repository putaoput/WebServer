//此头文件用来封装一些epoll的接口

#ifndef EPOLLAPI
#define EPOLLAPI

#include "requestData.h"

constexpr int MAX_EVENTS = 5000;
constexpr int LISTENQ = 1024;

int epoll_init();
int epoll_add(int, int, void* request, __uint32_t);
int epoll_mod(int, int, void* request, __uint32_t);
int my_epoll_wait(int epoll_fd, struct epoll_event*, int, int);

#endif // !EPOLLAPI
