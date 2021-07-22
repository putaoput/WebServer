# WebServer Linux高性能服务器

## 简介
这是一个基于C++11的高性能服务器。并发模型为Reactor+非阻塞IO+线程池。经过webbenchh压力测试可以同时支持上千用户的请求。

## 功能
* 使用Epoll边沿触发的IO多路复用技术
* 基于CAS技术和消费生产者模型无锁任务队列
* 智能指针，RAII，基于小根堆的定时器实现自动释放资源
* 异步日志记录功能以记录服务器工作情况
* 基于状态机的HTTP请求处理
* 简单连接MySQL数据库

## 版本介绍
1. 第一版：实现基本服务器功能
2. 第二版：引入智能指针和RAII技术管理资源
3. 第三版：异步日志与MySQL数据库的使用
4. 最新版：使用无锁任务队列替代线程池中的互斥锁

## 测试环境
 Ubuntu 16.04
 
## 测试结果
https://www.yuque.com/putaoput/xnow2a/qzoaw3

## 语雀文档
https://www.yuque.com/putaoput/xnow2a
