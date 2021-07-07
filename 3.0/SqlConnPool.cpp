//@Author Lin Tao
//@Email putaopu@qq.com

#include "SqlConnPool.h"
#include "Log.h"
using namespace std;

//-------------------------SqlConnPool的实现-------------------------
SqlConnPool::SqlConnPool() 
	:useCount(0),
	freeCount(0){}

SP_SqlConnPool SqlConnPool::instance() {
	SP_SqlConnPool SP_connPool(new SqlConnPool);
	return SP_connPool;
}
/*
   第四步：授权
    mysql>
    grant all privileges on *.* to 'root'@'localhost' identified by 'root' with grant option;
    1 use mysql;   然后敲回车
    2 update user set authentication_string=password("你的密码") where user="root";  然后敲回车
    3 flush privileges;  然后敲回车
    
    */

void SqlConnPool::init(const char* _host, int _port,
    const char* _user, const char* _pwd, const char* _dbName,
    int _connSize = 10) {
    assert(_connSize > 0);
    for (int i = 0; i < _connSize; i++) {
        MYSQL sql;
        //这里最好是自己管理一个对象，因为如果是交给mysql一个空指针的话，mysql会自己创建一个对象
        //但是如果调用mysql_close(),这个对象就会被销毁，然后如果该对象用在别处，就产生空悬指针
        MYSQL *psql = mysql_init(&sql);
        if (!psql) {
            LOG_ERROR("MySQL init error!");
           
            assert(psql);
        }
        
        psql = mysql_real_connect(psql, _host,
            _user, _pwd,
            _dbName, _port, nullptr, 0);
        if (!psql) {
            LOG_ERROR("MySql Connect error!");
             perror("MySQL Connect error!\n");
        }
        connQue.push(make_shared<MYSQL>(sql));
    }
    MAX_CONN = _connSize;
    sem_init(&semId, 0, MAX_CONN);
}

shared_ptr<MYSQL> SqlConnPool::get_conn() {
    shared_ptr<MYSQL> sql;

    if (connQue.empty()) {
        LOG_WARN("SqlConnPool busy!");   
    }else{
        sem_wait(&semId);//用来阻塞直到信号量大于零，成功后减一
        {
            //自定义锁的作用域，这样的优点是及时的释放RAII锁，这几乎是最快释放RAII锁的方式了
            //一种更慢的方法等到这个api结束之后自动释放锁。
            MutexLockGuard locker(Slock);
            sql = connQue.front();
            connQue.pop();
        }
    }
   
    return sql;
}

void SqlConnPool::free_conn(shared_ptr<MYSQL> sql) {
    assert(sql);
    MutexLockGuard locker(Slock);
    connQue.push(sql);
    sem_post(&semId);
}

void SqlConnPool::close_pool() {
    MutexLockGuard locker(Slock);
    while (!connQue.empty()) {
        auto item = connQue.front();
        connQue.pop();
        mysql_close(item.get());
    }
    mysql_library_end();//结束使用
}

int SqlConnPool::get_free_conn_count() {
    MutexLockGuard locker(Slock);
    return connQue.size();
}

/*
头文件
#include<mysql.h>
函数原型描述:
MYSQL *mysql_real_connect (MYSQL *mysql,
const char *host,
const char *user,
const char *passwd,
const char *db,
unsigned int port,
const char *unix_socket,
unsigned long client_flag)
上面描述了五个参数的主要取值，
MYSQL *为mysql_init函数返回的指针，
host为null或 localhost时链接的是本地的计算机，
当mysql默认安装在unix（或类unix）系统中，root账户是没有密码的，因此用户名使用root，密码为null，
当db为空的时候，函数链接到默认数据库，在进行 mysql安装时会存在默认的test数据库，因此此处可以使用test数据库名称，
port端口为0，
使用 unix连接方式，unix_socket为null时，表明不使用socket或管道机制，最后一个参数经常设置为0
mysql_real_connect()尝试与运行在主机上的MySQL数据库引擎建立连接。在你能够执行需要有效MySQL连接句柄结构的任何其他API函数之前，mysql_real_connect()必须成功完成。
返回值
如果连接成功，返回MYSQL*连接句柄。如果连接失败，返回NULL。对于成功的连接，返回值与第1个参数的值相同。

因此mysql_real_connect()函数调用为：
       mysql_real_connect(mysql,"localhost","root",NULL,"test",0,NULL,0);
判断是否出错，出错调用mysql_error()函数显示出错信息，或使用mysql_errno()函数获取出错代号




sem_init()函数详解
    信号量的数据类型为结构sem_t，它本质上是一个长整型的数。函数sem_init（）用来初始化一个信号量。它的原型为：

extern int sem_init __P ((sem_t *__sem, int __pshared, unsigned int __value));　　

sem为指向信号量结构的一个指针；

pshared不为０时此信号量在进程间共享，否则只能为当前进程的所有线程共享；

value给出了信号量的初始值。　　

 

函数sem_post( sem_t *sem )用来增加信号量的值。

当有线程阻塞在这个信号量上时，调用这个函数会使其中的一个线程不在阻塞，选择机制同样是由线程的调度策略决定的。　　

函数sem_wait( sem_t *sem )被用来阻塞当前线程直到信号量sem的值大于0，解除阻塞后将sem的值减一，表明公共资源经使用后减少。函数sem_trywait ( sem_t *sem )是函数sem_wait（）的非阻塞版本，它直接将信号量sem的值减一。　

　

函数sem_destroy(sem_t *sem)用来释放信号量sem。　

信号量用sem_init函数创建的，下面是它的说明：
#include
int sem_init (sem_t *sem, int pshared, unsigned int value);

    这个函数的作用是对由sem指定的信号量进行初始化，设置好它的共享选项，并指定一个整数类型的初始值。pshared参数控制着信号量的类型。如果 pshared的值是０，就表示它是当前里程的局部信号量；否则，其它进程就能够共享这个信号量。我们现在只对不让进程共享的信号量感兴趣。（这个参数受版本影响），pshared传递一个非零将会使函数调用失败。

这两个函数控制着信号量的值，它们的定义如下所示：　　
#include
int sem_wait(sem_t * sem);
int sem_post(sem_t * sem);
这两个函数都要用一个由sem_init调用初始化的信号量对象的指针做参数。
    sem_post函数的作用是给信号量的值加上一个“1”，它是一个“原子操作”－－－即同时对同一个信号量做加“1”操作的两个线程是不会冲突的；而同时对同一个文件进行读、加和写操作的两个程序就有可能会引起冲突。信号量的值永远会正确地加一个“2”－－因为有两个线程试图改变它。
    sem_wait函数也是一个原子操作，它的作用是从信号量的值减去一个“1”，但它永远会先等待该信号量为一个非零值才开始做减法。也就是说，如果你对一个值为2的信号量调用sem_wait(),线程将会继续执行，介信号量的值将减到1。如果对一个值为0的信号量调用sem_wait()，这个函数就会地等待直到有其它线程增加了这个值使它不再是0为止。如果有两个线程都在sem_wait()中等待同一个信号量变成非零值，那么当它被第三个线程增加一个“1”时，等待线程中只有一个能够对信号量做减法并继续执行，另一个还将处于等待状态。
信号量这种“只用一个函数就能原子化地测试和设置”的能力下正是它的价值所在。还有另外一个信号量函数sem_trywait，它是sem_wait的非阻塞搭档。

    最后一个信号量函数是sem_destroy。这个函数的作用是在我们用完信号量对它进行清理。下面的定义：
#include
int sem_destroy (sem_t *sem);
这个函数也使用一个信号量指针做参数，归还自己战胜的一切资源。在清理信号量的时候如果还有线程在等待它，用户就会收到一个错误。
与其它的函数一样，这些函数在成功时都返回“0”。

*/