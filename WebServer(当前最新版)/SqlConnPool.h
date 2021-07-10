//@Author Lin Tao
//@Email putaopu@qq.com
#pragma once

//�õ���ģʽ������SQL���ӳ�
//SQL���ӳ�ʹ��һ�����ȶ����������������MySQL���ݿ�֮�������
//�ṩһЩ�ӿڣ���
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
	sem_t semId;//ʹ���ź��������̼߳�ͨ�ţ�ע���������õ�ֵ��0��ֻ�иý��̵������߳̿���ʹ������ź���
	//���semId��ֵ���������ݿ����Ӷ������滹�������ɵ�SQLָ����е�ֵ��
};

#define  SP_SqlConnPool std::shared_ptr<SqlConnPool>

//�������RAII�ַ���װMYSQL��䣬ʵ����Դ���Զ��ͷ�
//��������Դ���Զ��ͷ�ָ�����Ұ�һ��MYSQL���ӷ���Sql����ص�ʱ�򣬿����Զ�������
//���������ǽ�MYSQL����������Ȼ���������ʱ���Զ��ӳ����ﵯ����
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
