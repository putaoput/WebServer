// @Author Lin Tao
// @Email putaopu@qq.com

#include "Log.h"
#include "config.h"
using namespace std;

Log::Log()
	:lineCount(0),
	isAsync(false),
	UPwriteThread(nullptr),
	UPdeque(nullptr),
	toDay(0),
	fp(nullptr),
	Llock(){}

	Log::~Log() {
		if (UPwriteThread && UPwriteThread->joinable()) {
			//要结束写写日志的线程
			while (!UPdeque->empty()) {
				UPdeque->flush();//唤醒消费者，目的是让所有没有写完的日志写入磁盘，写完只有UPdeque就会变空
			}
			UPdeque->close();

		}

		if (fp) {
			MutexLockGuard lock(Llock);
			flush();//如果是异步的话要flush UPdeque,保证数据都写入。然后fflush(fps)
					//flush:冲洗
			fclose(fp);
		}
	}

	int Log::get_level() {
		MutexLockGuard lock(Llock);
		return level;
}

	void Log::set_level(int _level) {
		MutexLockGuard lock(Llock);
		level = _level;
	}


	//init 函数负责log实例的初始化，也就是说要先调用LOG::instance(),然后调用init()进行初始化
	//instance的工作就是或得一个static Log
	void Log::init(int _level = 1, const char* _path,
		const char* _suffix, int _maxQueueSize) {
		//把isOpen置true，设置写入等级
		isOpen = true;
		level = _level;
		if (_maxQueueSize > 0) {
			isAsync = true;
			if (!UPdeque) {
				//新建一个blockdeque容器,然后用unique_ptr管理。
				unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
				UPdeque = move(newDeque);
				//新建一个写日志的线程
				std::unique_ptr<std::thread> newThread(new thread(flush_log_thread));
				UPwriteThread = move(newThread);
			}
		}
		else {
			isAsync = false;
		}

		lineCount = 0;

		time_t timer = time(nullptr);
		struct tm* sysTime = localtime(&timer);
		struct tm t = *sysTime;
		path = _path;
		suffix = _suffix;

		char fileName [LOG_NAME_LEN] = { 0 };
		/*
		* snprintf()，函数原型为int snprintf(char *str, size_t size, const char *format, ...)。
		*将可变参数 “…” 按照format的格式格式化为字符串，然后再将其拷贝至str中。
		*/

		//文件名是 年 + 月 + 日 + 前缀
		snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
			path, t.tm_year + 1890, t.tm_mon + 1, t.tm_mday, suffix);
		toDay = t.tm_mday;

		{
			MutexLockGuard lock(Llock);
			buff.retrieve_all();//全部重置,把整个buff清零
			if (fp) {
				flush();
				fclose(fp);
			}

			fp = fopen(fileName, "a");//"a" = "at"打开或者新建一个文件,只允许在末尾加。
			if (fp == nullptr) {
				mkdir(path, 0777);
				fp = fopen(fileName, "a");//创建一个新文件,并把该文件打开，准备好向该文件写入数据
			}
			assert(fp != nullptr);
		}
	}

	//write函数是主要对外接口
	void Log::write(int _level, const char* _format, ...) {
		struct timeval now = { 0,0 };
		gettimeofday(&now, nullptr);
		time_t tSec = now.tv_sec;
		struct tm* sysTime = localtime(&tSec);
		struct tm t = *sysTime;
		va_list vaList;

		//这一步是文件准备
		if (toDay != t.tm_mday || (lineCount && (lineCount % MAX_LINES == 0))) {

			//实现按天存储
			char newFile[LOG_NAME_LEN];
			char tail[36] = { 0 };
			snprintf(tail, 36, "04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

			if (toDay != t.tm_mday) {
				snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path, tail, suffix);
				toDay = t.tm_mday;
				lineCount = 0;
			}
			else {
				snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path, tail, (lineCount / MAX_LINES), suffix);
			}

			MutexLockGuard lock(Llock);
			flush();
			fclose(fp);
			fp = fopen(newFile, "a");
			assert(fp != nullptr);
		}

		//这一步是正式写入
		MutexLockGuard lock(Llock);
		++lineCount;//新启一行

		//首先向buff里写入时间信息
		int n = snprintf(buff.begin_write(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld",
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

		buff.has_written(n);
		//然后添加日志等级信息
		append_log_level(_level);

		va_start(vaList, _format);
		int m = vsnprintf(buff.begin_write(), buff.writable_bytes(), _format, vaList);
		//可变参数向字符串打印
		va_end(vaList);

		buff.has_written(m);
		buff.append("\n\0", 2);

		if (isAsync && UPdeque && !UPdeque->full()) {
			UPdeque->push_back(buff.retrieve_all_to_str());
		}
		else {
			fputs(buff.peek(), fp);
		}
		buff.retrieve_all();
	}


	//(level从0 - 3 分别为 debug,info, warn,error，默认是1：info)
	void Log::append_log_level(int _level) {
		switch (_level) {
		case 0:
			buff.append("[debug]: ", 9);
			break;
		case 1:
			buff.append("[info] :  ", 9);
			break;
		case 2:
			buff.append("[warn] :  ", 9);
			break;
		case 3:
			buff.append("[error]: ", 9);
			break;
		default:
			buff.append("[info] :  ",9);
		}
	}

	void Log::flush() {
		if (isAsync) {
			UPdeque->flush();
		}
		fflush(fp);//强迫缓冲区里的数据写入指定的流
	}

	//写日志的进程进行异步写
	void Log::async_write() {
		string str = "";
		while (UPdeque->pop(str)) {
			//在循环执行，只要封装的deque不为空，那么就pop出最前面的一个元素，然后写入文件
			MutexLockGuard lock(Llock);
			fputs(str.c_str(), fp);
		}
	}

	Log* Log::instance() {
		static Log inst;
		return &inst;
	}

	void Log::flush_log_thread() {
		Log::instance()->async_write();
	}

	bool Log::is_open(){
		return isOpen;
	}

