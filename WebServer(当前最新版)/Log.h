//@Author Lin Tao
//@Email putaopu@qq.com

#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "Buffer.h"
#include "MutexLock.h"
#include "LTSimpleSTL/thread_safe_deque.h"
class Log
{
public:
	void init(int _level, const char* path = "./log",
		const char* _suffix = ".log",
		int _maxQueueCapacity = 1024);

	static Log* instance();
	static void flush_log_thread();

	void write(int _level, const char* _format, ...);//三个点代表参数可变
	void flush();

	int get_level();
	void set_level(int _level);
	bool is_open();

private:
	Log();
	void append_log_level(int _level);
	virtual ~Log();
	void async_write();

	static const int LOG_PATH_LEN = 256;
	static const int LOG_NAME_LEN = 256;
	static const int MAX_LINES = 50000;

	const char* path;
	const char* suffix;

	//int MAX_LINES;

	int lineCount;
	int toDay;

	bool isOpen;

	Buffer buff;
	int level;
	bool isAsync;//是否异步 async异步，非同步

	FILE* fp;
	std::unique_ptr<LT::thread_safe_deque<std::string>> UPdeque;
	std::unique_ptr<std::thread> UPwriteThread;
	MutexLock Llock;
};


#define LOG_BASE(_level, _format,...)\
do{\
	Log*log = Log::instance();\
	if (log->is_open() && log->get_level() <= _level) {\
		log->write(_level,_format, ##__VA_ARGS__);\
		log->flush();\
	}\
} while (false);


#define LOG_DEBUG(_format,...) do{LOG_BASE(0,_format, ##__VA_ARGS__)}while(false);
#define LOG_INFO(_format,...) do{LOG_BASE(1,_format, ##__VA_ARGS__)}while(false);
#define LOG_WARN(_format,...) do{LOG_BASE(2,_format, ##__VA_ARGS__)}while(false);
#define LOG_ERROR(_format,...); do{LOG_BASE(3,_format, ##__VA_ARGS__)}while(false);

