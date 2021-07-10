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

