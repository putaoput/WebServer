// @Author Lin Tao
// @Email putaopu@qq.com

#include "ThreadPool.h"
#include "config.h"
#include <iostream>

using namespace std;

//-------------------------ThreadPoolTask------------------------

ThreadPoolTask::ThreadPoolTask(shared_ptr<Task> _task)
	:args(_task)
{
	fun = [](shared_ptr<Task> _task)->void {
#ifdef TEST
		cout << "_task.use_count() = " << _task.use_count() << endl;
#endif // TEST

		_task->receive();
	};
}

ThreadPoolTask::ThreadPoolTask()
{
	fun = [](shared_ptr<Task> _task)->void {
	#ifdef TEST
		cout << "TEST FUNC SUCCESS\n"<<endl;
		
	#endif
	_task->receive();
	};
	args = nullptr;
}

////自定义拷贝函数，交换智能指针
//ThreadPoolTask::ThreadPoolTask(const ThreadPoolTask& _task) {
//	_task.get_args()
//}

void ThreadPoolTask::reset() {
	args.reset();//STL中reset的内部实现就是新建一个空的智能指针，然后交换他们所指的对象。
}

void ThreadPoolTask::swap(ThreadPoolTask& _newTask) {
	args.swap(_newTask.args);
	#ifdef TEST
		cout << "swap" << args.use_count() << endl;
	#endif
}


//-------------------------MyThread--------------------------
//MyThread::MyThread()
//	:Tlock(),
//	Tnotify(Tlock),
//	thread(bind(thread_running,this),"MyThread")//传入线程要运行的函数和线程的名字
//{}
//
//MyThread::~MyThread()
//{
//	if (SPloop) {
//		SPloop->quit();
//		thread.join();
//	}
//}
//
//SP_EventLoop MyThread::start_loop() {
//	thread.start();
//	MutexLockGuard lock(Tlock);
//	// 一直等到threadFun在Thread里真正跑起来
//	while (SPloop)
//	{
//		Tnotify.wait();
//	}
//	return SPloop;
//}
//
//void MyThread::thread_running() {
//	MutexLockGuard lock(Tlock);
//	SPloop = make_shared<EventLoop>(new EventLoop);
//	Tnotify.notify();
//
//	SPloop->loop();
//	SPloop.reset();
//}



//-------------------------ThreadPool------------------------

MutexLock ThreadPool::TPlock;
Condition ThreadPool::TPnotify(TPlock);
std::vector<pthread_t> ThreadPool::threads;
std::vector<shared_ptr<ThreadPoolTask>> ThreadPool::taskQueue;
int ThreadPool::threadCount = 0;
int ThreadPool::queueSize = MAX_QUEUE;
int ThreadPool::head = 0;
int ThreadPool::shutdwon = 0;
int ThreadPool::started = 0;
bool ThreadPool::isValid = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;

int ThreadPool::create(int _threadCount, int _queueSize)//int _threadCount, int _queueSize) 

{
#ifdef TEST
	cout << "ThreadPoll constuct!!" << endl;
	cout << _queueSize << endl;
#endif 
	if (_threadCount <= 0 || _threadCount > MAX_THREADS) {
		printf("num of threads error!!!\n");
		return -1;
	}
	else {
		threads.resize(_threadCount);
	}


	if (_queueSize <= 0 || _queueSize > MAX_QUEUE) {
		printf("num of threads error!!!\n");
		return -1;
	}
	else {
		for (int i = 0; i < _queueSize; ++i) {
#ifdef TEST
			cout << "taskQueue constuct!!" << endl;
#endif 
			taskQueue.push_back(shared_ptr<ThreadPoolTask>(new ThreadPoolTask));

		}
	}

	/* 启动线程池里的线程 */
	/*
			创建新线程的函数：
			int pthread_create(pthread_t* thread, const pthread_attr_t* arr,
			void* ( *start_routine)(void*), void* arg);
			thread参数是新线程的标识符
			arr == NULL表示使用默认新线程属性
			第三个参数是线程运行函数的起始地址。
			最后一个参数是运行函数的参数。
	*/
	for (int i = 0; i < _threadCount; ++i) {
		if (pthread_create(&threads[i], NULL, running, (void*)(0)) != 0)
		{
			return -1;
		}
		++started;
		++threadCount;
	}

	isValid = true;
	return 0;
}

int ThreadPool::add(shared_ptr<ThreadPoolTask> _newTask) {
#ifdef TEST
	cout << "add new Task" << endl;
	//测试有没有和定时器分离成功
	if (_newTask->args->get_singleManager()->separate_success() == false) {
		cout << "separate failed!!" << endl;
	}
#endif

	/*if (pthread_mutex_lock(&lock) != 0) {
		perror("lock failed while add new task!!!\n");
		return -1;
	}*/
	MutexLockGuard lock(TPlock);
	if (count == queueSize) {
		perror("taskQueue is full!!!\n");
		return -1;
	}

	if (shutdwon) {
		perror("PthreadPool shutdown!!!\n");
		return -1;
	}

#ifdef TEST
	cout << "have locked while adding new Task" << endl;
#endif

	int next = (tail + 1) % queueSize;
	taskQueue[tail] = _newTask;//使用交换的方法加入任务队列
	++count;
	tail = next;
#ifdef PTHREAD
	cout << "next = " << next << "  count = " << count << endl;
#endif
	//唤醒等待该互斥量的线程
	TPnotify.notify();
	/*if (pthread_cond_signal(&notify) != 0)
	{
		perror("pthread_cond_signal failed after add new task!!!\n");
		return -1;
	}
	

	if (pthread_mutex_unlock(&lock) != 0) {
		perror("pthread_mutex_unlock failed after add new task!!!\n");
		return -1;
	}*/


#ifdef TEST
	_newTask->args->get_fd();
	cout << "end add new Task" << endl;
#endif

	return 0;
}

//void ThreadPool:: shutdown_pool(int _shutdwon) {
	//ThreadPool::shutdwon = _shutdown;
//}

void* ThreadPool::running(void* args) {

	while (true) {
#ifdef PTHREAD
		cout << "start()" << endl;
		cout << "count() = " << count << endl;
#endif
		/*pthread_mutex_lock(&lock);*/
		MutexLockGuard lock(TPlock);
		while ((count == 0) && (!shutdwon)) {
			TPnotify.wait();
			//pthread_cond_wait(&notify, &lock);
			//不满足给定条件时，让线程挂起
		}
#ifdef PTHREAD
		cout << "end wait!!" << endl;

#endif
		//检测到关闭信号则关闭
		if ((shutdwon == IMMEDIATE_SHUTDOWN) ||
			(shutdwon == WAIT_SHUTDOWN && !count)) {
			break;
		}
#ifdef PTHREAD
		cout << "head = " << head << endl;
#endif
		shared_ptr<ThreadPoolTask> task = taskQueue[head];
		taskQueue[head].reset();
		head = (head + 1) % queueSize;
		//!!重大失误，没有队列里的task,没有--count;
		--count;

		//pthread_mutex_unlock(&lock);

#ifdef TEST
		cout << "" << task->args->get_fd() << endl;
		cout << "before (task.fun)(task.args)" << endl;

#endif

		(task->fun)(task->args);
#ifdef PTHREAD
		cout << "after (task.fun)(task.args)" << endl;
#endif

#ifdef PTHREAD
		cout << "after task.reset()" << endl;
#endif
	}

	//如果退出线程
	MutexLockGuard lock (TPlock);
	--started;
	//pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
	return (NULL);
}

bool ThreadPool::is_valid() {
	return isValid;
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

//创建互斥量cond：
//
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER
//
//还可以通过函数的方式创建：
//
//int pthread_cond_init(pthread_cond_t * cond, NULL);
//
//注意：两种方式创建有所区别，第一种是创建静态全局的，所创建的量在全局区，
//第二种创建的量在栈，两种的区别我们可以理解为全局变量和局部变量的差异，
//如果创建的是全局的条件量则用第一种方式初始化，如果是在函数内部创建则用第二种方式。
//这里其实跟互斥锁一样，互斥锁也可以用这两种方式初始化，原理相同。

//int pthread_cond_destroy(pthread_cond_t* cond);
//
//注意：只有在没有线程使用该互斥量的时候才能销毁，否则会返回EBUSY。
//
//等待：
//
//当线程不满足给定条件时，让线程挂起，挂起分为两种，一种是无限制的等待，pthread_cond_wait；
//一种是定时等待，pthread_cond_timedwait。无论是哪种等待都必须和锁配合使用
//，防止多个线程同时使用互斥量。
//
//注意两个函数
//
//pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);
//
//pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* time);
//
//
//唤醒：
//
//当线程满足某个条件时，可以唤醒被挂起的线程，唤醒有两个函数
//
//一是唤醒等待该互斥量的线程：
//
//pthread_cond_signal(pthread_cond_t* cond);
//
//二是唤醒所有等待线程：
//
//pthread_cond_broadcast(pthread_cond_t* cond);

	//void pthread_exit(void* retval);
	//通过retval参数想线程的回收者传递其退出信息，该函数执行完毕之后不会返回到调用者，而且永远不会失败。