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
#include "Log.h"
#include "SqlConnPool.h"

using namespace std;

//-----------------------------Task------------------------
//初始化Task的时候，要提供等待处理的文件描述符，在程序启动，0是标准输入，1是标准输出，2是标准错误，所以新打开一个文件，
//他的文件描述符会是3，因为POSIX标准要求每次新打开一个文件时，必须使用目前最小可用的文件描述符号码)

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
		sp_timer = nullptr;
	}
}

//析构时要释放fd
Task::~Task()
{
	close(fd);
#ifdef _EPOLL_
	cout << "~Task()" << endl;
#endif
}

void Task::reset()
{
	againTimes = 0;
	message.clear();
	fileName.clear();
	path.clear();
	h_state = H_START;
	state = PARSE_URI;
	readPos = 0;
	keep_alive = false;
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
	char buff[BUFF_SIZE];
	while (true)
	{	
		int readNum = readn(buff, BUFF_SIZE);
		if (readNum < 0)
		{
			perror("fail to read!!!\n");
			isError = STATE_ERROR;
			break;
			//这个时候直接break会快一些，不然的话进入到了state_machine多出一些判断
		}
		else if (readNum == 0)//注意，封装readn时，在遇到EINTER这一关闭信号时，任然直接返回0.
		{
			perror("read but read_num == 0");
			if (errno == EAGAIN)//暂时没有数据可读，非阻塞情况下是直接返回的。
			{
#ifdef _EPOLL_
		cout << "again times = " << againTimes << endl;
#endif 
				if (againTimes > AGAIN_MAX_TIMES)
				{
					isError = STATE_ERROR;
				}
				else
				{
					++againTimes;
					//超过了次数说明可能是关闭了连接，此时服务器也要关闭这个socket连接。
				}
				
			}else if (errno != 0)
			{
				//这一情况时读取出现了错误
				isError = STATE_ERROR;
			}
			break;//注意这里也要break
		}
		message += string(buff, buff + readNum);
#ifdef _WEB_
		cout << "message = " << message << endl;
#endif
		state_machine();
		//跳出循环的条件
		if (isError == STATE_ERROR	        //此时将直接返回，释放
			|| isError == STATE_AGAIN	    //此时不会改变状态，然后重新加入
			|| state == FINISH				//正常结束同下
			|| isError == STATE_SUCCESS)    //此时重置状态，长连接重新加入，短连接同错误，直接释放
		{
#ifdef _EPOLL_
	cout << "break !! isError = " << isError << endl;
#endif
			break;
		}
	}

	if (isError == STATE_ERROR)
	{
		return -1;
	}

	if (state == FINISH)
	{
		if (keep_alive)
		{
#ifdef KA
	cout << "reset() for long connect!!!" << endl;
#endif
			reset();
		}else{
			return -1;
		}
	}

	//接下来就是把该请求放入该对象所属时间管理器了
	//要新建一个时间管理器管理该请求
	//要重置该事件
	shared_ptr<SingleTimer> sp_tiemr(new SingleTimer(TIME_OUT));
	set_singleManager(sp_tiemr);
	timerManager->add(sp_tiemr);

	//然后注册该事件
	if (myEpoll == nullptr)
	{
		perror("myEpoll is nullptr!!!\n");
		return -1;
	}
	events = EPOLLIN | EPOLLET | EPOLLONESHOT;

	myEpoll->mod((shared_from_this()));
	//加入任务队列
	myEpoll->add_to_epollTask(shared_from_this());

	return 0;
}

void Task::state_machine()
{
	if (state == PARSE_URI)
	{
		parse_uri();
#ifdef TEST
		cout << "state == PARSE_URI";
#endif
	}

	if (state == PARSE_HEADERS)
	{
		parse_headers();
#ifdef TEST
		cout << "state == PARSE_HEADERS";
#endif
	}

	if (state == RECV_BODY)
	{
		recv_body();
#ifdef TEST
		cout << "state == RECV_BODY";
#endif
	}

	if (state == ANALYSIS)
	{
		analysis();
#ifdef TEST
		cout << "state == ANALYSIS";
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
			}else{
				fileName = "index.html";
			}
		}
		pos = _pos;
	}

	// HTTP 版本号
#ifdef _MYSQL_
	cout << " HTTPVersion" << endl;
#endif

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
#ifdef _MYSQL_
	cout << " end prase uri" << endl;
#endif

}

void Task::parse_headers()
{
#ifdef _MYSQL_
	cout << " start parse headers" << endl;
#endif
	string &str = message;
	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;

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
			break;
		}
		case H_SPACE:
		{
			h_state = H_VALUE;
			value_start = i;
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
			break;
		}
		case H_FINAL_N:
		{
			notFinish = false;
			key_start = i;
			now_read_line_begin = i;
			break;
		}
		}
	}
	if (h_state == H_FINAL_N)
	{
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
		return;
	}
	str = str.substr(now_read_line_begin);
	isError = STATE_AGAIN;
#ifdef _MYSQL_
	cout << " end parse headers, isError = " << isError << endl;
#endif
}

void Task::analysis()
{
#ifdef _MYSQL_
	cout << " start analysis" << endl;
#endif
	if(method != METHOD_POST && method != METHOD_GET){
		isError = STATE_ERROR;
		return;
	}
		char header[BUFF_SIZE];
		sprintf(header, "HTTP/1.1 200 OK\r\n");
		keep_alive = false;
		if (headers.find("Connection") != headers.end() )
		{
			if ((headers["Connection"] == "keep-alive")|| (headers["Connection"] == "Keep-Alive")) {
				keep_alive = true;
			}
			sprintf(header, "%sConnection: %s\r\n", header, headers["Connection"]);
			sprintf(header, "%s%s: timeout=%d\r\n", header, headers["Connection"], EPOLL_WAIT_TIME);
		}
		//这个用来返回类型信息
		int dotPos = fileName.find('.');
		const char *filetype;
		if (dotPos < 0){
			filetype = MimeType::instance()->get_mime("default").c_str();
		}	
		else{
			filetype = MimeType::instance()->get_mime(fileName.substr(dotPos)).c_str();
			fileName = fileName.substr(0,dotPos);
		}
#ifdef _MYSQL_
		cout << "filetype = " << filetype << endl;
#endif
//这里需要一个filename的转换函数，通过filename来寻找对应网页。
#ifdef _WEB_
		cout << "fileName = :"<< fileName << endl;
#endif 
		fileName = "./pages/" + WebPage::instance()->get_page(fileName);
#ifdef _WEB_
		cout << "fileName = :"<< fileName << endl;
		cout << "header = " << header << endl;
#endif // _MYSQL
		struct stat sbuf;
		if (stat(fileName.c_str(), &sbuf) < 0)
		{
			//检查目标文件是否存在
			handle_error(404, "Not Found!");
			isError = STATE_ERROR;
			return;
		}

		sprintf(header, "%sContent-type: %s\r\n", header, filetype);
		// 通过Content-length返回文件大小
		sprintf(header, "%sContent-length: %ld\r\n", header, sbuf.st_size);

		sprintf(header, "%s\r\n", header);

		size_t sendLen = (size_t)writen(header, strlen(header));

		if (sendLen != strlen(header))
		{
			perror("Send header failed");
			isError = STATE_ERROR;
			return;
		}
		
		int src_fd = open(fileName.c_str(), O_RDONLY, 0);
		
		assert(src_fd);
		char *src_addr = static_cast<char *>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		//mmap将文件直接印射到内存以提高文件访问速度，或者是使用sendfile执行零拷贝操作
		close(src_fd);

		// 发送文件并校验完整性
		sendLen = writen(src_addr, sbuf.st_size);
		if (sendLen != sbuf.st_size)
		{
			perror("Send file failed");
			isError = STATE_ERROR;
			return;
		}
		munmap(src_addr, sbuf.st_size);
		isError = STATE_SUCCESS;
		state = FINISH;
		LOG_INFO("thread send a page to client:%d",fd);	
}

void Task::recv_body()
{
	int messageLength = -1;
	if (headers.find("Content-length") != headers.end())
	{
		messageLength = stoi(headers["Content-length"]);
		postBody = message;
		message.clear();
#ifdef _MYSQL_
		cout << "posBody.size() = " << postBody.size() << " messageLength = " << messageLength << endl;
#endif // _MYSQL_
	}
	else
	{
		isError = STATE_ERROR;
		return;
	}
	if (postBody.size() >= messageLength)
	{
		state = ANALYSIS;
	}
}

void Task::handle_error(int _errNum, string _shortMsg)
{
	_shortMsg = " " + _shortMsg;
	char send_buff[BUFF_SIZE];
	string bodybuff, header_buff;
	bodybuff += "<html><title>TKeed Error</title>";
	bodybuff += "<body bgcolor=\"ffffff\">";
	bodybuff += to_string(_errNum) + _shortMsg;
	bodybuff += "<hr><em> LinTao's Web Server</em>\n</body></html>";

	header_buff += "HTTP/1.1 " + to_string(_errNum) + _shortMsg + "\r\n";
	header_buff += "Content-type: text/html\r\n";
	header_buff += "Connection: close\r\n";
	header_buff += "Content-length: " + to_string(bodybuff.size()) + "\r\n";
	header_buff += "\r\n";
	sprintf(send_buff, "%s", header_buff.c_str());
	writen(send_buff, strlen(send_buff));
	sprintf(send_buff, "%s", bodybuff.c_str());
	writen(send_buff, strlen(send_buff));
}

ssize_t Task::readn(void* _buff, size_t _n) {
  size_t needToRead = _n;
  ssize_t readOnce = 0;
  ssize_t readSum = 0;
  char *ptr = (char *)_buff;
  while (needToRead > 0) {
    if ((readOnce = read(fd, ptr, needToRead)) < 0) {
      if (errno == EINTR){
        readOnce = 0;
	  }
      else if (errno == EAGAIN) {
        return readSum;
		//连续做read操作而没有数据可读。此时程序不会阻塞起来等待数据准备就绪返回，
		//read函数会返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。
		//之前没有return 所以就在读完数据之后不断的读数据，导致错误
      } else {
        return -1;
      }
    } else if (readOnce == 0)
      break;
    readSum += readOnce;
    needToRead -= readOnce;
    ptr += readOnce;
  }
  return readSum;
}

ssize_t Task::writen(void* _buff, size_t _n)
{
	size_t needToWrite = _n;
	ssize_t writeOnce = 0;
	ssize_t writeSum = 0;
	char* ptr = (char*)_buff;
	while (needToWrite > 0)
	{
		if ((writeOnce = write(fd, ptr, needToWrite)) <= 0)
			//if ((writeOnce == send(fd, ptr, needToWrite,MSG_DONTWAIT)) < 0)
		{
			if(writeOnce < 0){
			if (errno == EINTR )
			{
				//如果进程在一个慢系统调用(slow system call)中阻塞时，当捕获到某个信号且相应信号处理函数返回时，
				//这个系统调用被中断,调用返回错误，设置errno为EINTR（相应的错误描述为“Interrupted system call”）。
				writeOnce = 0;
				continue;
			}
			else if (errno == EAGAIN) {
				return writeSum;
			}
			else
			{
				return -1;
			}
		}
		}
		writeSum += writeOnce;
		needToWrite -= writeOnce;
		ptr += writeOnce;
	}
	return writeSum;
}

/*
关于客户端关闭tcp连接之后发生的事情
这里面socket连接是非阻塞的。
对于阻塞的socket，当socket的接收缓冲区中没有数据时，read调用会一直阻塞住，直到有数据到来，
非阻塞的socket无论有没有数据，read都将立即返回。
对于写操作也类似。
如果对方关闭了socket连接的话，read返回0，write返回EPIPE,这是唯一的信号(非阻塞+ socket情况)
*/
//
void Task::parse_post() {
	//最常见的 POST 提交数据的方式。浏览器的原生 <form> 表单，如果不设置 enctype 属性，那么最终就会以 application / x - www - form - urlencoded 方式提交数据
	if (method ==  METHOD_POST && headers["Content-Type"] == "application/x-www-form-urlencoded") {
		parse_from_urlencoded();
		if (fileName.find("register") != -1 || fileName.find("login") != -1) {
			if (user_verify(postMap["username"], postMap["password"], fileName.find("login") != -1)) {
				fileName = "welcome.html";
			}
			else {
				fileName = "error.html";
			}
		}		
	}
}

void Task::parse_from_urlencoded(){
	if (postBody.size() == 0) { return; }

	string key, value;
	int num = 0;
	int n = postBody.size();
	int i = 0, j = 0;

	for (; i < n; i++) {
		char ch = postBody[i];
		switch (ch) {
		case '=':
			key = postBody.substr(j, i - j);
			j = i + 1;
			break;
		case '+':
			postBody[i] = ' ';
			break;
		case '%':
			num = conver(postBody[i + 1]) * 16 + conver(postBody[i + 2]);
			postBody[i + 2] = num % 10 + '0';
			postBody[i + 1] = num / 10 + '0';
			i += 2;
			break;
		case '&':
			value = postBody.substr(j, i - j);
			j = i + 1;
			postMap[key] = value;
			LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
			break;
		default:
			break;
		}
	}
	assert(j <= i);
	if (postMap.count(key) == 0 && j < i) {
		value = postBody.substr(j, i - j);
		postMap[key] = value;
	}
}

bool Task::user_verify(const string& _name, const string& _pwd, bool _isLogin) {
	if (_name == "" || _pwd == "") { return false; }
	LOG_INFO("Verify _name:%s _pwd:%s", _name.c_str(), _pwd.c_str());
	shared_ptr<MYSQL> SPsql(new MYSQL);
	MysqlGuard(SPsql, SqlConnPool::instance());
	assert(SPsql);

	bool flag = false;
	unsigned int j = 0;
	char order[256] = { 0 };
	MYSQL_FIELD* fields = nullptr;
	MYSQL_RES* res = nullptr;

	if (!_isLogin) { flag = true; }
	/* 查询用户及密码 */
	snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", _name.c_str());
	LOG_DEBUG("%s", order);

	if (mysql_query(SPsql.get(), order)) {
		mysql_free_result(res);
		return false;
	}
	res = mysql_store_result(SPsql.get());
	j = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);

   //mysql_fetch_row是一个同步函数，用来获取结果集的下一行
	while (MYSQL_ROW row = mysql_fetch_row(res)) {
		LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
		string password(row[1]);
		/* 登录行为 且 用户名未被使用*/
		if (_isLogin) {
			if (_pwd == password) { flag = true; }
			else {
				flag = false;
				LOG_DEBUG("pwd error!");
			}
		}
		else {
			flag = false;
			LOG_DEBUG("user used!");
		}
	}
	mysql_free_result(res);

	/* 注册行为 且 用户名未被使用*/
	if (!_isLogin && flag == true) {
		LOG_DEBUG("regirster!");
		bzero(order, 256);
		snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", _name.c_str(), _pwd.c_str());
		LOG_DEBUG("%s", order);
		if (mysql_query(SPsql.get(), order)) {
			LOG_DEBUG("Insert error!");
			flag = false;
		}
		flag = true;
	}
	LOG_DEBUG("UserVerify success!!");
	return flag;
}

int Task::conver(char ch) {
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	return ch;
}
