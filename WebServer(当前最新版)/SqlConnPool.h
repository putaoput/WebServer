//@Author Lin Tao
//@Email putaopu@qq.com
#pragma once

//用单例模式构建的SQL连接池
//SQL连接池使用一个优先队列来管理服务器和MySQL数据库之间的连接
//提供一些接口，把
#include <mysql/mysql.h>
#include <string>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <memory>
#include <queue>
#include <cassert>

#include "MutexLock.h"
#include "Condition.h"

class MysqlGuard;

class SqlConnPool
{
public:
	static std::shared_ptr<SqlConnPool> instance();

	std::shared_ptr<MYSQL> get_conn();
	int get_free_conn_count();

	void init(const char* _host, int _port, const char* _user, const char* _pwd,
		const char* _dbName, int _conSize);
	void close_pool();
	~SqlConnPool(){
		close_pool();
	}
private:
	friend MysqlGuard;
	void free_conn(std::shared_ptr<MYSQL> _conn);
	SqlConnPool();
	

	int MAX_CONN;
	int useCount;
	int freeCount;

	std::queue<std::shared_ptr<MYSQL>> connQue;
	MutexLock Slock;
	sem_t semId;//使用信号量进行线程间通信，注意这里设置的值是0，只有该进程的所有线程可以使用这个信号量
	//这个semId的值代表了数据库连接队列里面还可以容纳的SQL指令队列的值。
};

#define  SP_SqlConnPool std::shared_ptr<SqlConnPool>

//这个类用RAII手法封装MYSQL语句，实现资源的自动释放
//这里面资源的自动释放指的是我把一个MYSQL连接放入Sql任务池的时候，可以自动弹出，
//创建对象是将MYSQL语句放入池子里，然后对象析构时会自动从池子里弹出。
struct MysqlGuard {
	MysqlGuard(std::shared_ptr<MYSQL> _sql, SP_SqlConnPool _connPool){
		assert(_connPool);
		_sql = _connPool->get_conn();
		sql = _sql;
		connPool = _connPool;
	}

	~MysqlGuard() {
		if (sql) {
			connPool->free_conn(sql);
		}
	}
private:
	SP_SqlConnPool connPool;
	std::shared_ptr<MYSQL> sql;
};
