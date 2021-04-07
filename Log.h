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
#include "blockdeque.h"
#include "MutexLock.h"

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
	std::unique_ptr<BlockDeque<std::string>> UPdeque;
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
#define LOG_ERROR(_format,...) do{LOG_BASE(3,_format, ##__VA_ARGS__)}while(false);

/*
* # ## __VA_ARGS__ 和 ##__VA_ARGS__的作用
一、#用来把参数转换成字符串

例子一：

 

#define P(A) printf("%s:%d\n",#A,A);

int main(int argc, char **argv)

{
        int a = 1, b = 2;

        P(a);

        P(b);

        P(a+b);

        system("pause");

}

输出为：



 

例子二：

#define SQUARE(x) printf("The square of "#x" is %d.\n", ((x)*(x)));

 

SQUARE(8)

输出的是：The square of 8 is 64

 

二、##运算符可以用于宏函数的替换部分。这个运算符把两个语言符号组合成单个语言符号，为宏扩展提供了一种连接实际变元的手段

 

例子一：

#define XNAME(n) x ## n

 

如果这样使用宏：XNAME(8)

则会被展开成这样：x8

 

##就是个粘合剂，将前后两部分粘合起来，也就是有“字符化”的意思。但是“##”不能随意粘合任意字符，必须是合法的C语言标示符。在单一的宏定义中，最多可以出现一次“#”或“##”预处理操作符。如果没有指定与“#”或“##”预处理操作符相关的计算次序，则会产生问题。为避免该问题，在单一的宏定义中只能使用其中一种操作符(即，一份“#”或一个“##”，或都不用)。除非非常有必要，否则尽量不要使用“#”和“##”。

 

三、__VA_ARGS__ 是一个可变参数的宏，很少人知道这个宏，这个可变参数的宏是新的C99规范中新增的，目前似乎只有gcc支持（VC6.0的编译器不支持）。
实现思想就是宏定义中参数列表的最后一个参数为省略号（也就是三个点）。

 

四、##__VA_ARGS__ 宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错

一般这个用在调试信息上多一点

例如：

 

#define my_print1(...)  printf(__VA_ARGS__)   my_print1("i=%d,j=%d\n",i,j)  正确打印

#define my_print2(fmt,...)  printf(fmt,__VA_ARGS__)  

my_print1("i=%d,j=%d\n",i,j) 正确打印

my_print2("iiiiiii\n")       编译失败打印，因为扩展出来只有一个参数，至少要两个及以上参数

如果是#define my_print2(fmt,...)  printf(fmt,##__VA_ARGS__)  

那么

my_print1里面不管是几个参数都能正确打印
*/