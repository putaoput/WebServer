#ifndef THREADPOOL
#define TFREADPOOL
#include"requestData.h"
#include<pthread.h>
//linux下多线程库，遵循POSIX线程接口

//定义一些错误提示的宏
constexpr int THREADPOOL_INVALID = -1;
constexpr int THREADPOOL_LOCK_FAILURE = -2;
constexpr int THREADPOOL_QUEUE_FULL = -3;
constexpr int THREADPOOL_SHUTDOWN = -4;
constexpr int THREADPOOL_THREAD_FAILURE = -5;
constexpr int THREADPOOL_GRACEFUL = 1;


//定义最大线程数和最大队列大小
constexpr int MAX_THREADS = 1024;
constexpr int MAX_QUEUE = 65535;

typedef enum
{
	immediate_shutdown = 1,
	graceful_shutdown = 2 //优雅的关闭
} threadpool_shutdown_t;

typedef struct 
{
	void (*function)(void*);
	void* argument;
}threadpool_task_t;

/*
该结构体上面存的是函数指针，
下面存的是输入改函数指针的参数
*/
struct threadpool_t
{
	pthread_mutex_t lock;
	pthread_cond_t notify;
	pthread_t* threads; //这是一个存储线程标识的数组
	threadpool_task_t* queue;
	int thread_count;
	int queue_size;
	int head;
	int tail;
	int count;
	int shutdown;
	int started;

};

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */


threadpool_t* threadpool_create(int , int , int);
int threadpool_add(threadpool_t*, void(*function)(void*), void*, int);
//void*(*func)(void *):函数指针，因为c语言无法像c++一样，把一个仿函数的地址传给一个函数
//第一个void*是函数的类型，而函数是一个无名的函数，第二个void*是函数的形参。

int threadpool_destroy(threadpool_t* pool, int flags); 
int threadpool_free(threadpool_t* pool);
static void* threadpool_thread(void* threadpool);
#endif // THREAD
