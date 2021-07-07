//这一文件主要存放了操作线程池的一些封装函数

#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

//①新建一个线程池
threadpool_t* threadpool_create(int thread_count, int queue_size, int flags)
{
	threadpool_t* pool;
	//声明一个指向 定义在threadpool.h头文件中的自定义线程池,这也是要返回的东西

	//先检查输入参数是否正确
	if (thread_count <= 0 || thread_count > MAX_THREADS)
	{
		perror("thread_count error!!");
		return NULL;
	}
	if (queue_size <= 0 || queue_size > MAX_QUEUE)
	{
		perror("queue_size error!!");
		return NULL;
	}

	if ((pool = (threadpool_t*)malloc(sizeof(threadpool_t))) == NULL)
	{
		//分配内存失败
		perror("malloc failed!!");
		threadpool_free(pool);
		//析构定义的threadpool
		return NULL;
	}

	//分配内存成功之后初始化定义的结构体里的各种变量
	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = 0;
	pool->tail = 0;
	pool->count = 0;
	pool->shutdown = 0;
	pool->started = 0;

	//使用c语言的方式分配线程和任务队列
	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
	pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

	//对线程池里的互斥锁进行和条件进行初始化
	if ((pthread_mutex_init(&(pool->lock), NULL) != 0)
		|| (pthread_cond_init(&(pool->notify), NULL) != 0)
		|| (pool->threads == NULL)
		|| (pool->queue == NULL))
	{
		//锁
		perror("pthread_mutex_init failed!!");
		threadpool_free(pool);
		//析构定义的threadpool
		return NULL;
	}

	/*
		对互斥锁mutex的介绍
		#include <pthread.h>
		int pthread_mutex_init(pthread_mutex_t *mutex, const phread_mutexattr_t mutexattr );
		//mutex指向要操作的目标锁，pthread_mutex_t是一个结构体
		//mutexattr指明互斥锁的属性，NULL表明使用默认属性，默认属性下是普通锁
		//当一个线程对普通锁加锁之后，其余请求该锁的线程将形成一个等待队列，并在该锁被解锁是按优先级获得它
		问题:一个线程对一个已经加锁的普通锁加锁，将引发死锁
			一个已经解锁的普通锁再次解锁，将导致不可预期的后果

		条件变量:用在线程之间同步共享数据的值，条件变量提供了一种机制，当共享数据达到某个值的时候，
				唤醒等待这个共享数据的线程
		pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* cond_attr);
				第一个参数cond指向将要操作的目标条件变量，条件变量类型是pthread_cond_t结构体
				cond_attr指定条件变量的属性。如果设置为NULL表示使用默认时薪
	*/

	for (int i = 0; i < thread_count; ++i) 
	{
		if (pthread_create((&pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0)
		{
			/*
			创建新线程的函数：
			int pthread_create(pthread_t* thread, const pthread_attr_t* arr,
			void* ( *start_routine)(void*), void* arg);
			thread参数是新线程的标识符
			arr == NULL表示使用默认新线程属性
			第三个参数是线程运行函数的起始地址。
			最后一个参数是运行函数的参数。
			*/

			//创建新线程失败，线程池里面每一个线程都是被事先创建好
			threadpool_destroy(pool, 0);
			return NULL;
		}
		++pool->thread_count;
		++pool->started;
	}

	return pool;
}

//②摧毁线程池
/*
包括一下几个步骤:
	指针判空;
	加锁
	以广播方式唤醒所有等待目标条件变量的进程，解锁？？？这里不太懂
	用一个for循环，和pthread_join函数结束线程池中的每一个线程
*/
int threadpool_destroy(threadpool_t* pool, int flags)
{
	printf("Tread pool destroy! \n");

	int err = 0;
	if (pool == NULL)
	{
		return THREADPOOL_INVALID;
	}

	if (pthread_mutex_lock(&(pool->lock)) != 0)
	{
		//pthread_mutex_lock以原子操作的方式给一个互斥锁加锁,如果该锁已经被锁住，pthread_mutex_lock调用将阻塞，
		//直到该锁的占有者将其解锁
		return THREADPOOL_LOCK_FAILURE;
	}

	do //使用do来避免多次重复同一个return
	{
		if (pool->shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}

		pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;

		//唤醒所有工作线程
		if ((pthread_cond_broadcast(&(pool->notify)) != 0)
			|| (pthread_mutex_unlock(&(pool->lock)) != 0))
		{
			//pthread_cond_broadcast以广播方式唤醒所有等待目标条件变量的线程
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}

		for (int i = 0; i < pool->thread_count; ++i)
		{
			if (pthread_join(pool->threads[i], NULL) != 0)
			{
				err = THREADPOOL_THREAD_FAILURE;
			}
			//一个进程中的所有线程都可以调用pthread_join函数来回收其他线程
			//第一个参数是目标线程的标识符，第二个参数是目标线程返回的退出信息pthread_join(pthread_t thread,void**retval);
			//该函数会一直被阻塞直到被回收的线程结束为止
			//成功是返回0
		}
	} while (false);
	
	if (!err)
	{
		threadpool_free(pool);
	}
}

//释放线程池，有时候判出线程池为空，也会调用这个函数
/*
	销毁前四个成员变量
*/
int threadpool_free(threadpool_t* pool)
{
	//第一个，老样子，先判指针是否为空和已经开始的线程数是否不为零
	if (pool == NULL || pool->started > 0)
	{
		return -1;
	}

	if (pool->threads)
	{
		free(pool->threads);
		free(pool->queue);

		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		//销毁互斥锁
		pthread_cond_destroy(&(pool->notify));
		//用于销毁条件变量，销毁一个正在等待的条件变量将失败把并返回EBUSY;
	}
	free(pool);
	return 0;
}

//这是创建线程池里每一个新线程后，每个新线程都执行该静态函数
static void* threadpool_thread(void* threadpool)
{
	threadpool_t* pool = (threadpool_t*)threadpool;
	threadpool_task_t task;

	/*
	typedef struct 
{
	void (*function)(void*);
	void* argument;
}threadpool_task_t;
	*/

	//创建线程后，程序就在一直执行这个for循环，参数存在线程池里。
	for (;;)
	{
		//从线程池中获取参数时，首先要给互斥锁上锁
		 pthread_mutex_lock(&(pool->lock));

		//其次检查线程池中待执行任务数是否非零，线程池是否需要被关闭
		 while ((pool->count == 0) && (!pool->shutdown))
		 {
			 //等待条件变量满足后，线程就开始执行
			 pthread_cond_wait(&(pool->notify), &(pool->lock));
			 //pthread_cond_wait用于等待目标条件变量(pthread_cond_t* cond,pthread_mutex_t * mutex);
			 //第二个参数是用于保护目标条件变量的互斥锁，使用时必须保证该互斥锁已经被加锁
		 }

		 if ((pool->shutdown == immediate_shutdown) ||
			 (pool->shutdown == graceful_shutdown && pool->count == 0))
		 {
			 //如果发现该线程需要立刻关闭，或者是慢关闭(等待线程池中任务数都执行完再关闭)
			 break;
		 }
		 

		 /* Grab our task */
		 task.function = pool->queue[pool->head].function;
		 //pool->queue里存的是指向自定义的threadpool_task的指针
		 task.argument = pool->queue[pool->head].argument;
		 //function是一个函数指针
		 pool->head = (pool->head + 1) % pool->queue_size;//在队列里循环
		 pool->count -= 1;

		 /*Unlock*/
		 pthread_mutex_unlock(&(pool->lock));

		 
		 //获得任务后执行任务
		 cout << "task!" << endl;
		 (*(task.function))(task.argument);
	}
	//如果该线程退出了，一开始线程计数减1。
	--pool->started;

	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);

	//void pthread_exit(void* retval);
	//通过retval参数想线程的回收者传递其退出信息，该函数执行完毕之后不会返回到调用者，而且永远不会失败。

	return(NULL);
}

int threadpool_add(threadpool_t *pool, void(*function)(void*), void *argument, int flags)
{
	int err = 0;
	int next;
	if(pool == NULL || function == NULL)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	//!@@! 巨大的BUG出处，忘记加锁了
	if ((pthread_mutex_lock(&pool->lock)) != 0)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	next = (pool->tail + 1) % pool->queue_size;
	do
	{
		if(pool->count == pool->queue_size)
		{
			err = THREADPOOL_QUEUE_FULL;
			break;
		}
		
		if(pool->shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		pool->queue[pool->tail].function = function;
		pool->queue[pool->tail].argument = argument;
		pool->tail = next;
		pool->count += 1;
		
		if(pthread_cond_signal(&(pool->notify)) != 0)
		{
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
		
	}while(false);

	if(pthread_mutex_unlock(&pool->lock) != 0)
	{
		err = THREADPOOL_LOCK_FAILURE;
	}

	return err;
}
