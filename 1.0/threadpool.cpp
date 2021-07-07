//��һ�ļ���Ҫ����˲����̳߳ص�һЩ��װ����

#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

//���½�һ���̳߳�
threadpool_t* threadpool_create(int thread_count, int queue_size, int flags)
{
	threadpool_t* pool;
	//����һ��ָ�� ������threadpool.hͷ�ļ��е��Զ����̳߳�,��Ҳ��Ҫ���صĶ���

	//�ȼ����������Ƿ���ȷ
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
		//�����ڴ�ʧ��
		perror("malloc failed!!");
		threadpool_free(pool);
		//���������threadpool
		return NULL;
	}

	//�����ڴ�ɹ�֮���ʼ������Ľṹ����ĸ��ֱ���
	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = 0;
	pool->tail = 0;
	pool->count = 0;
	pool->shutdown = 0;
	pool->started = 0;

	//ʹ��c���Եķ�ʽ�����̺߳��������
	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
	pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

	//���̳߳���Ļ��������к��������г�ʼ��
	if ((pthread_mutex_init(&(pool->lock), NULL) != 0)
		|| (pthread_cond_init(&(pool->notify), NULL) != 0)
		|| (pool->threads == NULL)
		|| (pool->queue == NULL))
	{
		//��
		perror("pthread_mutex_init failed!!");
		threadpool_free(pool);
		//���������threadpool
		return NULL;
	}

	/*
		�Ի�����mutex�Ľ���
		#include <pthread.h>
		int pthread_mutex_init(pthread_mutex_t *mutex, const phread_mutexattr_t mutexattr );
		//mutexָ��Ҫ������Ŀ������pthread_mutex_t��һ���ṹ��
		//mutexattrָ�������������ԣ�NULL����ʹ��Ĭ�����ԣ�Ĭ������������ͨ��
		//��һ���̶߳���ͨ������֮����������������߳̽��γ�һ���ȴ����У����ڸ����������ǰ����ȼ������
		����:һ���̶߳�һ���Ѿ���������ͨ������������������
			һ���Ѿ���������ͨ���ٴν����������²���Ԥ�ڵĺ��

		��������:�����߳�֮��ͬ���������ݵ�ֵ�����������ṩ��һ�ֻ��ƣ����������ݴﵽĳ��ֵ��ʱ��
				���ѵȴ�����������ݵ��߳�
		pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* cond_attr);
				��һ������condָ��Ҫ������Ŀ��������������������������pthread_cond_t�ṹ��
				cond_attrָ���������������ԡ��������ΪNULL��ʾʹ��Ĭ��ʱн
	*/

	for (int i = 0; i < thread_count; ++i) 
	{
		if (pthread_create((&pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0)
		{
			/*
			�������̵߳ĺ�����
			int pthread_create(pthread_t* thread, const pthread_attr_t* arr,
			void* ( *start_routine)(void*), void* arg);
			thread���������̵߳ı�ʶ��
			arr == NULL��ʾʹ��Ĭ�����߳�����
			�������������߳����к�������ʼ��ַ��
			���һ�����������к����Ĳ�����
			*/

			//�������߳�ʧ�ܣ��̳߳�����ÿһ���̶߳��Ǳ����ȴ�����
			threadpool_destroy(pool, 0);
			return NULL;
		}
		++pool->thread_count;
		++pool->started;
	}

	return pool;
}

//�ڴݻ��̳߳�
/*
����һ�¼�������:
	ָ���п�;
	����
	�Թ㲥��ʽ�������еȴ�Ŀ�����������Ľ��̣��������������ﲻ̫��
	��һ��forѭ������pthread_join���������̳߳��е�ÿһ���߳�
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
		//pthread_mutex_lock��ԭ�Ӳ����ķ�ʽ��һ������������,��������Ѿ�����ס��pthread_mutex_lock���ý�������
		//ֱ��������ռ���߽������
		return THREADPOOL_LOCK_FAILURE;
	}

	do //ʹ��do���������ظ�ͬһ��return
	{
		if (pool->shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}

		pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;

		//�������й����߳�
		if ((pthread_cond_broadcast(&(pool->notify)) != 0)
			|| (pthread_mutex_unlock(&(pool->lock)) != 0))
		{
			//pthread_cond_broadcast�Թ㲥��ʽ�������еȴ�Ŀ�������������߳�
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}

		for (int i = 0; i < pool->thread_count; ++i)
		{
			if (pthread_join(pool->threads[i], NULL) != 0)
			{
				err = THREADPOOL_THREAD_FAILURE;
			}
			//һ�������е������̶߳����Ե���pthread_join���������������߳�
			//��һ��������Ŀ���̵߳ı�ʶ�����ڶ���������Ŀ���̷߳��ص��˳���Ϣpthread_join(pthread_t thread,void**retval);
			//�ú�����һֱ������ֱ�������յ��߳̽���Ϊֹ
			//�ɹ��Ƿ���0
		}
	} while (false);
	
	if (!err)
	{
		threadpool_free(pool);
	}
}

//�ͷ��̳߳أ���ʱ���г��̳߳�Ϊ�գ�Ҳ������������
/*
	����ǰ�ĸ���Ա����
*/
int threadpool_free(threadpool_t* pool)
{
	//��һ���������ӣ�����ָ���Ƿ�Ϊ�պ��Ѿ���ʼ���߳����Ƿ�Ϊ��
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
		//���ٻ�����
		pthread_cond_destroy(&(pool->notify));
		//����������������������һ�����ڵȴ�������������ʧ�ܰѲ�����EBUSY;
	}
	free(pool);
	return 0;
}

//���Ǵ����̳߳���ÿһ�����̺߳�ÿ�����̶߳�ִ�иþ�̬����
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

	//�����̺߳󣬳������һֱִ�����forѭ�������������̳߳��
	for (;;)
	{
		//���̳߳��л�ȡ����ʱ������Ҫ������������
		 pthread_mutex_lock(&(pool->lock));

		//��μ���̳߳��д�ִ���������Ƿ���㣬�̳߳��Ƿ���Ҫ���ر�
		 while ((pool->count == 0) && (!pool->shutdown))
		 {
			 //�ȴ���������������߳̾Ϳ�ʼִ��
			 pthread_cond_wait(&(pool->notify), &(pool->lock));
			 //pthread_cond_wait���ڵȴ�Ŀ����������(pthread_cond_t* cond,pthread_mutex_t * mutex);
			 //�ڶ������������ڱ���Ŀ�����������Ļ�������ʹ��ʱ���뱣֤�û������Ѿ�������
		 }

		 if ((pool->shutdown == immediate_shutdown) ||
			 (pool->shutdown == graceful_shutdown && pool->count == 0))
		 {
			 //������ָ��߳���Ҫ���̹رգ����������ر�(�ȴ��̳߳�����������ִ�����ٹر�)
			 break;
		 }
		 

		 /* Grab our task */
		 task.function = pool->queue[pool->head].function;
		 //pool->queue������ָ���Զ����threadpool_task��ָ��
		 task.argument = pool->queue[pool->head].argument;
		 //function��һ������ָ��
		 pool->head = (pool->head + 1) % pool->queue_size;//�ڶ�����ѭ��
		 pool->count -= 1;

		 /*Unlock*/
		 pthread_mutex_unlock(&(pool->lock));

		 
		 //��������ִ������
		 cout << "task!" << endl;
		 (*(task.function))(task.argument);
	}
	//������߳��˳��ˣ�һ��ʼ�̼߳�����1��
	--pool->started;

	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);

	//void pthread_exit(void* retval);
	//ͨ��retval�������̵߳Ļ����ߴ������˳���Ϣ���ú���ִ�����֮�󲻻᷵�ص������ߣ�������Զ����ʧ�ܡ�

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
	//!@@! �޴��BUG���������Ǽ�����
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
