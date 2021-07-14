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
			//Ҫ����дд��־���߳�
			while (!UPdeque->empty()) {
				UPdeque->flush();//���������ߣ�Ŀ����������û��д�����־д����̣�д��ֻ��UPdeque�ͻ���
			}
			UPdeque->close();

		}

		if (fp) {
			MutexLockGuard lock(Llock);
			flush();//������첽�Ļ�Ҫflush UPdeque,��֤���ݶ�д�롣Ȼ��fflush(fps)
					//flush:��ϴ
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


	//init ��������logʵ���ĳ�ʼ����Ҳ����˵Ҫ�ȵ���LOG::instance(),Ȼ�����init()���г�ʼ��
	//instance�Ĺ������ǻ��һ��static Log
	void Log::init(int _level = 1, const char* _path,
		const char* _suffix, int _maxQueueSize) {
		//��isOpen��true������д��ȼ�
		isOpen = true;
		level = _level;
		if (_maxQueueSize > 0) {
			isAsync = true;
			if (!UPdeque) {
				//�½�һ��thread_safe_deque����,Ȼ����unique_ptr����
				unique_ptr<LT::thread_safe_deque<std::string>> newDeque(new LT::thread_safe_deque<std::string>);
				UPdeque = move(newDeque);
				//�½�һ��д��־���߳�
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
		* snprintf()������ԭ��Ϊint snprintf(char *str, size_t size, const char *format, ...)��
		*���ɱ���� ������ ����format�ĸ�ʽ��ʽ��Ϊ�ַ�����Ȼ���ٽ��俽����str�С�
		*/

		//�ļ����� �� + �� + �� + ǰ׺
		snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
			path, t.tm_year + 1890, t.tm_mon + 1, t.tm_mday, suffix);
		toDay = t.tm_mday;

		{
			MutexLockGuard lock(Llock);
			buff.retrieve_all();//ȫ������,������buff����
			if (fp) {
				flush();
				fclose(fp);
			}

			fp = fopen(fileName, "a");//"a" = "at"�򿪻����½�һ���ļ�,ֻ������ĩβ�ӡ�
			if (fp == nullptr) {
				mkdir(path, 0777);
				fp = fopen(fileName, "a");//����һ�����ļ�,���Ѹ��ļ��򿪣�׼��������ļ�д������
			}
			assert(fp != nullptr);
		}
	}

	//write��������Ҫ����ӿ�
	void Log::write(int _level, const char* _format, ...) {
		struct timeval now = { 0,0 };
		gettimeofday(&now, nullptr);
		time_t tSec = now.tv_sec;
		struct tm* sysTime = localtime(&tSec);
		struct tm t = *sysTime;
		va_list vaList;

		//��һ�����ļ�׼��
		if (toDay != t.tm_mday || (lineCount && (lineCount % MAX_LINES == 0))) {

			//ʵ�ְ���洢
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

		//��һ������ʽд��
		MutexLockGuard lock(Llock);
		++lineCount;//����һ��

		//������buff��д��ʱ����Ϣ
		int n = snprintf(buff.begin_write(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld",
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

		buff.has_written(n);
		//Ȼ�������־�ȼ���Ϣ
		append_log_level(_level);

		va_start(vaList, _format);
		int m = vsnprintf(buff.begin_write(), buff.writable_bytes(), _format, vaList);
		//�ɱ�������ַ�����ӡ
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


	//(level��0 - 3 �ֱ�Ϊ debug,info, warn,error��Ĭ����1��info)
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
		fflush(fp);//ǿ�Ȼ������������д��ָ������
	}

	//д��־�Ľ��̽����첽д
	void Log::async_write() {
		string str = "";
		while (UPdeque->pop(str)) {
			//��ѭ��ִ�У�ֻҪ��װ��deque��Ϊ�գ���ô��pop����ǰ���һ��Ԫ�أ�Ȼ��д���ļ�
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

