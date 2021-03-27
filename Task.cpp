// @Author Lin Tao
// @Email putaopu@qq.com

#include <unistd.h> //write,read所在头文件
#include <sys/time.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "config.h"
#include "Task.h"
#include "Mime.h"
#include "MyEpoll.h"
#include "SingerTimer.h"
#include "TimerManager.h"
//#include "util.h"

using namespace std;

//-----------------------------Task------------------------
//初始化Task的时候，要提供等待处理的文件描述符

Task::Task(int _fd, shared_ptr<MyEpoll> _myEpoll, size_t _timeOut, string _path, shared_ptr<TimerManager> _timerManager, __uint32_t _events)
	: timerManager(_timerManager),
	  state(PARSE_URI),
	  fd(_fd),
	  myEpoll(_myEpoll),
	  h_state(H_START),
	  keep_alive(false),
	  path(_path),
	  readPos(0),
	  events(_events)
{
#ifdef TEST
	cout << "Task Construct!!!" << fd << endl;
#endif
}

void Task::set_singleManager(std::shared_ptr<SingleTimer> _sp_timer)
{
	timer = _sp_timer;
}

void Task::separate()
{
	if (timer.lock())
	{
		shared_ptr<SingleTimer> sp_timer(timer.lock());
		sp_timer->separate_task();
	}
}

//重载构造函数,
//Task::Task( size_t _timeOut, shared_ptr<TimerManager> _timerManager):
//	SingleTimer( _timerManager, _timeOut),
//	state(PARSE_URI),
//	h_state(H_START),
//	keep_alive(false),
//	readPos(0),
//	isError(0)
//{

//}

//析构时要释放fd
Task::~Task()
{
#ifdef TEST
	cout << "~Task()" << endl;
#endif
	shared_ptr<Task> _task = shared_from_this();
	myEpoll->del(static_pointer_cast<Task>(shared_from_this()));

	//这里为了加速运行，所以使用静态转型，由我自己保证安全性~~~~
}
//重写reset(),重写之后继承的SingleTimer里面的时间信息不会变，保证可以被成功弹出。
void Task::reset()
{
	againTimes = 0;
	message.clear();
	fileName.clear();
	path.clear();
	state = H_START;
	readPos = 0;
	keep_alive = false;
	//SingleTimer::reset();
}

int Task::get_fd()
{
	return fd;
}

__uint32_t Task::get_events()
{
	return events;
}

int Task::receive()
{
#ifdef TEST
	cout << "receive!!" << this->get_fd() << endl;
#endif
	char buff[BUFF_SIZE];
	while (true)
	{
		//int readNum = readn(buff,BUFF_SIZE);
		int readNum = readn(fd, buff, BUFF_SIZE);
#ifdef TEST
		cout << "readNum = " << readNum << endl;
#endif
		if (readNum < 0)
		{
			perror("fail to read!!!\n");
			isError = true;
		}
		else if (readNum == 0)
		{
			perror("read but read_num == 0");
			if (errno == EAGAIN)
			{
				if (againTimes > AGAIN_MAX_TIMES)
				{
					isError = STATE_ERROR;
				}
				else
				{
					++againTimes;
				}
			}
			else if (errno != 0)
			{
				isError = true;
			}
		}
		message += string(buff, buff + readNum);
#ifdef TEST
		cout << "message = " << message << endl;
#endif

		state_machine();

#ifdef TEST
		cout << "After state_machine!!" << endl;
#endif

		//跳出循环的条件
		if (isError == STATE_ERROR	   //此时将直接返回，释放
			|| isError == STATE_AGAIN	   //此时不会改变状态，然后重新加入
			|| state == FINISH
			|| isError == STATE_SUCCESS) //此时重置状态，长连接重新加入，短连接同错误，直接释放
		{
			break;
		}
	}

	if (isError)
	{
		return -1;
	}
	if (state == FINISH)
	{
		if (keep_alive)
		{
			reset();
		}
		else
		{
			return -1;
		}
	}

	//接下来就是把该请求放入该对象所属时间管理器了
	//要新建一个时间管理器管理该请求
	//要重置该事件
	shared_ptr<SingleTimer> sp_tiemr(new SingleTimer(TIME_OUT));
	set_singleManager(sp_tiemr);
	timerManager->add(sp_tiemr);
	reset();

	//然后注册该事件
	if (myEpoll == nullptr)
	{
		perror("myEpoll is nullptr!!!\n");
		return -1;
	}
	events = EPOLLIN | EPOLLET | EPOLLONESHOT;

#ifdef TEST
	cout << "epoll->mod()" << endl;
#endif // TEST

	myEpoll->mod(dynamic_pointer_cast<Task>(shared_from_this()));
}

ssize_t Task::readn(void *_buff, size_t _n)
{
	size_t needToRead = _n;
	ssize_t readOnce = 0;
	ssize_t readSum = 0;
	char *ptr = (char *)_buff;

	while (needToRead > 0)
	{
#ifdef TEST
		cout << "readn fd = " << fd << endl;
#endif
		if ((readOnce == read(fd, ptr, needToRead)) < 0)
		{

			if (errno == EINTR || errno == EAGAIN)
			{
				readOnce = 0;
				continue;
			}
			else
			{
				return -1;
			}
		}
#ifdef TEST
		cout << "readOnce = " << readOnce << endl;
#endif
		readSum += readOnce;
		needToRead -= readOnce;
		ptr += readOnce;
	}
	return readSum;
}

ssize_t Task::wrriten(int _fd,void *_buff, size_t _n)
{
	size_t needToWrite = _n;
	ssize_t writeOnce = 0;
	ssize_t writeSum = 0;
	char *ptr = (char *)_buff;
	while (needToWrite > 0)
	{
#ifdef TEST
		cout << "needToWrite = " << needToWrite << endl;
#endif
		if ((writeOnce == write(_fd, ptr, needToWrite)) < 0)
		//if ((writeOnce == send(fd, ptr, needToWrite,MSG_DONTWAIT)) < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				writeOnce = 0;
				continue;
			}
			else
			{
				return -1;
			}
		}
		writeSum += writeOnce;
		needToWrite -= writeOnce;
		ptr += writeOnce;
	}
	return writeSum;
}

void Task::state_machine()
{

	if (state == PARSE_URI)
	{
		parse_uri();
#ifdef TEST
		cout << "PARSE_URI" << endl;
		if (state == PARSE_URI)
		{
			cout << "start parse_headers" << endl;
		}
#endif
	}

	if (state == PARSE_HEADERS)
	{
		parse_headers();
#ifdef TEST
		cout << "PARSE_HREADERS " << endl;
#endif
	}

	if (state == RECV_BODY)
	{
		recv_body();
#ifdef TEST
		cout << "RECV_BODY" << endl;
#endif
	}

	if (state == ANALYSIS)
	{
		analysis();
#ifdef TEST
		cout << "ANALYSIS" << endl;
#endif
	}
}

void Task::parse_uri()
{
	string &str = message;
	// 读到完整的请求行再开始解析请求
	int pos = str.find('\r', readPos);
	if (pos < 0)
	{
		isError = STATE_AGAIN;
		return;
	}
	// 去掉请求行所占的空间，节省空间
	string requestLine = str.substr(0, pos);
	if (str.size() > pos + 1)
		str = str.substr(pos + 1);
	else
		str.clear();
	// Method
	pos = requestLine.find("GET");
	if (pos < 0)
	{
		pos = requestLine.find("POST");
		if (pos < 0)
		{
			isError = STATE_ERROR;
			return;
		}
		else
		{
			method = METHOD_POST;
			return;
		}
	}
	else
	{
		method = METHOD_GET;
	}

	pos = requestLine.find("/", pos);
	if (pos < 0)
	{
		isError = STATE_ERROR;
		return;
	}
	else
	{
		int _pos = requestLine.find(' ', pos);
		if (_pos < 0)
			return;
		else
		{
			if (_pos - pos > 1)
			{
				fileName = requestLine.substr(pos + 1, _pos - pos - 1);
				int __pos = fileName.find('?');
				if (__pos >= 0)
				{
					fileName = fileName.substr(0, __pos);
				}
			}

			else
				fileName = "index.html";
		}
		pos = _pos;
	}

	// HTTP 版本号
	pos = requestLine.find("/", pos);
	if (pos < 0)
	{
		isError = STATE_ERROR;
		return;
	}
	else
	{
		if (requestLine.size() - pos <= 3)
		{
			isError = STATE_ERROR;
			return;
		}
		else
		{
			string ver = requestLine.substr(pos + 1, 3);
			if (ver == "1.0")
				httpVersion = HTTP_10;
			else if (ver == "1.1")
				httpVersion = HTTP_11;
			else
				return;
		}
	}
	state = PARSE_HEADERS;
	isError = STATE_SUCCESS;
}

void Task::parse_headers()
{
	string &str = message;
	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;
	//#ifdef TEST
	//cout << "start parse_headers" << endl;
	//if(h_state != H_START){
	//	cout << "h_state != H_START, = " << h_state << endl;
	//}
	//#endif
	for (int i = 0; i < str.size() && notFinish; ++i)
	{
		switch (h_state)
		{
		case H_START:
		{
			if (str[i] == '\n' || str[i] == '\r')
				break;
			h_state = H_KEY;
			key_start = i;
			now_read_line_begin = i;
#ifdef TEST
			//cout << "H_START" << endl;
#endif
			break;
		}
		case H_KEY:
		{
			if (str[i] == ':')
			{
				key_end = i;
				if (key_end - key_start <= 0)
				{
					isError = STATE_ERROR;
					return;
				}

				h_state = H_COLON;
			}
			else if (str[i] == '\n' || str[i] == '\r')
			{
				isError = STATE_ERROR;
				return;
			}
#ifdef TEST
			//cout << "2" << endl;
#endif
			break;
		}
		case H_COLON:
		{
			if (str[i] == ' ')
			{
				h_state = H_SPACE;
			}
			else
			{
				isError = STATE_ERROR;
				return;
			}
#ifdef TEST
			//cout << "3" << endl;
#endif
			break;
		}
		case H_SPACE:
		{
			h_state = H_VALUE;
			value_start = i;
#ifdef TEST
			//cout << "4" << endl;
#endif
			break;
		}
		case H_VALUE:
		{
			if (str[i] == '\r')
			{
				h_state = H_R;
				value_end = i;
				if (value_end - value_start <= 0)
				{
					isError = STATE_ERROR;
					return;
				}
			}
			else if (i - value_start > 255)
			{
				isError = STATE_ERROR;
				return;
			}

#ifdef TEST
			//cout << "5" << endl;
#endif
			break;
		}
		case H_R:
		{
			if (str[i] == '\n')
			{
				h_state = H_N;
				string key(str.begin() + key_start, str.begin() + key_end);
				string value(str.begin() + value_start, str.begin() + value_end);
				headers[key] = value;
				now_read_line_begin = i;
			}
			else
			{
				isError = STATE_ERROR;
				return;
			}
#ifdef TEST
			//cout << "6" << endl;
#endif

			break;
		}
		case H_N:
		{
			if (str[i] == '\r')
			{
				h_state = H_FINAL_R;
			}
			else
			{
				key_start = i;
				h_state = H_KEY;
			}

#ifdef TEST
			//cout << "7" << endl;
#endif
			break;
		}
		case H_FINAL_R:
		{
			if (str[i] == '\n')
			{
				h_state = H_FINAL_N;
			}
			else
			{
				isError = STATE_ERROR;
				return;
			}
#ifdef TEST
			//cout << "8" << endl;
#endif
			break;
		}
		case H_FINAL_N:
		{
			notFinish = false;
			key_start = i;
			now_read_line_begin = i;
#ifdef TEST
			//cout << "9" << endl;
#endif

			break;
		}
		}
	}
	if (h_state == H_FINAL_N)
	{
#ifdef TEST
		cout << "h_state == H_FINAL_N" << endl;
		cout << now_read_line_begin << " " << str.size() << endl;
#endif
		str = str.substr(now_read_line_begin);

		isError = STATE_SUCCESS;
		if (method == METHOD_POST)
		{
			state = RECV_BODY;
		}
		else
		{
			state = ANALYSIS;
		}
#ifdef TEST
		cout << "state = " << state << endl;
#endif
		return;
	}
#ifdef TEST
	cout << "h_state != H_FINAL_N" << endl;
#endif

	str = str.substr(now_read_line_begin);
	isError = STATE_AGAIN;
}

void Task::analysis()
{
#ifdef TEST
	for (const auto &pair : headers)
	{
		cout << "key = " << pair.first << "  value = " << pair.second << endl;
	}

#endif
	if (method == METHOD_POST)
	{

		char header[BUFF_SIZE];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if (headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive")
		{
			keep_alive = true;
			sprintf(header, "%sConnection: keep-alive\r\n", header);
			sprintf(header, "%skeep-alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
		}

		char *sendMessage = "I have receiced this.";

		sprintf(header, "%sContent-length: %zu\r\n", header, strlen(sendMessage));
		sprintf(header, "%s\r\n", header);
		size_t sendLen = (size_t)writen(fd, header, strlen(header));
		if (sendLen != strlen(header))
		{
			perror("Send header failed");
			isError = true;
			return;
		}

		sendLen = (size_t)writen(fd,(void*)sendMessage, strlen(sendMessage));
		if (sendLen != strlen(sendMessage))
		{
			perror("Send message failed");
			isError = true;
			return;
		}

		isError = STATE_SUCCESS;
	}
	else if (method == METHOD_GET)
	{
		char header[BUFF_SIZE];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if (headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive")
		{
#ifdef TEST
			cout << "keep_alive" << endl; //很奇怪，有时候大写，有时小写
#endif
			keep_alive = true;
			sprintf(header, "%sConnection: keep-alive\r\n", header);
			sprintf(header, "%skeep-alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
		}

		int dotPos = fileName.find('.');
		const char *filetype;
		if (dotPos < 0)
			filetype = MimeType::getMime("default").c_str();
		else
			filetype = MimeType::getMime(fileName.substr(dotPos)).c_str();
		struct stat sbuf;
		if (stat(fileName.c_str(), &sbuf) < 0)
		{
			handle_error(404, "Not Found!");
			isError = STATE_ERROR;
			return;
		}

		sprintf(header, "%sContent-type: %s\r\n", header, filetype);
		// 通过Content-length返回文件大小
		sprintf(header, "%sContent-length: %ld\r\n", header, sbuf.st_size);

		sprintf(header, "%s\r\n", header);
#ifdef TEST
		cout << "start write!!" << endl;
#endif
		size_t sendLen = (size_t)writen(fd,header, strlen(header));
#ifdef TEST
		cout << "sendLen = " << sendLen << endl;
#endif
		if (sendLen != strlen(header))
		{
			perror("Send header failed");
			isError = STATE_ERROR;
			return;
		}
		int src_fd = open(fileName.c_str(), O_RDONLY, 0);
		char *src_addr = static_cast<char *>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		close(src_fd);

#ifdef TEST
		cout << "sendheader:   " << header << endl;
#endif
		// 发送文件并校验完整性
		sendLen = writen(fd, src_addr, sbuf.st_size);
		if (sendLen != sbuf.st_size)
		{
			perror("Send file failed");
			isError = STATE_ERROR;
			return;
		}
		munmap(src_addr, sbuf.st_size);
		isError = STATE_SUCCESS;
		state = FINISH;
	}
	else
	{
		isError = STATE_ERROR;
		return;
	}
}

void Task::recv_body()
{
	int messageLength = -1;
	if (headers.find("Content-length") != headers.end())
	{
		messageLength = stoi(headers["Content-length"]);
	}
	else
	{
		isError = STATE_ERROR;
		return;
	}
	if (message.size() < messageLength)
	{
		state = ANALYSIS;
	}
}

void Task::handle_error(int _errNum, string _shortMsg)
{
	_shortMsg = " " + _shortMsg;
	char send_buff[BUFF_SIZE];
	string body_buff, header_buff;
	body_buff += "<html><title>TKeed Error</title>";
	body_buff += "<body bgcolor=\"ffffff\">";
	body_buff += to_string(_errNum) + _shortMsg;
	body_buff += "<hr><em> LinTao's Web Server</em>\n</body></html>";

	header_buff += "HTTP/1.1 " + to_string(_errNum) + _shortMsg + "\r\n";
	header_buff += "Content-type: text/html\r\n";
	header_buff += "Connection: close\r\n";
	header_buff += "Content-length: " + to_string(body_buff.size()) + "\r\n";
	header_buff += "\r\n";
	sprintf(send_buff, "%s", header_buff.c_str());
	writen(fd, send_buff, strlen(send_buff));
	sprintf(send_buff, "%s", body_buff.c_str());
	writen(fd, send_buff, strlen(send_buff));
}

ssize_t Task::readn(int fd, void *buff, size_t n)
{
	size_t nleft = n;
	ssize_t nread = 0;
	ssize_t readSum = 0;
	char *ptr = (char *)buff;
	while (nleft > 0)
	{
		//if ((nread = read(fd, ptr, nleft)) < 0)
		if ((nread = recv(fd, ptr, n, 0)) < 0)
		{
			if (errno == EINTR)
				nread = 0;
			else if (errno == EAGAIN)
			{
				return readSum;
			}
			else
			{
				return -1;
			}
		}
		else if (nread == 0)
			break;
		readSum += nread;
		nleft -= nread;
		ptr += nread;
	}
	return readSum;
}

ssize_t Task::writen(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;

#ifdef TEST
    cout << n << endl;
#endif

    char *ptr = (char*)buff;
   while (nleft > 0)
   if(nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR || errno == EAGAIN)
                {
                    nwritten = 0;
                    //continue;
                }
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}