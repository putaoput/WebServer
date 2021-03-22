// @Author Lin Tao
// @Email putaopu@qq.com

#include <unistd.h> //write,read����ͷ�ļ�
#include <sys/time.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>

#include"config.h"
#include"Task.h"
#include "Mime.h"
#include <fcntl.h>
#include "MyEpoll.h"
#include <iostream>

using namespace std;

//-----------------------------Task------------------------
//��ʼ��Task��ʱ��Ҫ�ṩ�ȴ��������ļ�������

Task::Task(int _fd, shared_ptr<MyEpoll> _myEpoll, size_t _timeOut, string _path, shared_ptr<TimerManager> _timerManager, __uint32_t _events)
	:SingleTimer(_timerManager,_timeOut),
	state(PARSE_URI),
	fd(_fd),
	myEpoll(_myEpoll),
	h_state(H_START),
	keep_alive(false),
	path(_path),
	readPos(0),
	events(_events)
{

}



//���ع��캯��,
//Task::Task( size_t _timeOut, shared_ptr<TimerManager> _timerManager):
//	SingleTimer( _timerManager, _timeOut),
//	state(PARSE_URI),
//	h_state(H_START),
//	keep_alive(false),
//	readPos(0),
//	isError(0)
//{

//}

//����ʱҪ�ͷ�fd
Task::~Task() {
	#ifdef TEST
	cout << "~Task()" << endl;
	#endif
	myEpoll->del(static_pointer_cast<Task>(shared_from_this()));
		//����Ϊ�˼������У�����ʹ�þ�̬ת�ͣ������Լ���֤��ȫ��~~~~
}
//��дreset()
void Task::reset() {
	againTimes = 0;
	message.clear();
	fileName.clear();
	path.clear();
	state = H_START;
	readPos = 0;
	keep_alive = false;
	SingleTimer::reset();
}

int Task::get_fd() {
	return fd;
}

__uint32_t Task::get_events() {
	return events;
}

int Task::receive() {
	#ifdef TEST
		cout << "receive!!" << endl;
	#endif
	char buff[BUFF_SIZE];
	while (true) {
		int readNum = readn(buff);
		#ifdef TEST
		cout << "readNum = " << readNum << endl;
		#endif
		if (readNum < 0) {
			perror("fail to read!!!\n");
			isError = true;
		}
		else if (readNum == 0) {
			perror("read but read_num == 0");
			if (errno == EAGAIN) {
				if (againTimes > AGAIN_MAX_TIMES) {
					isError = STATE_ERROR;
				}
				else {
					++againTimes;
				}
			}
			else if (errno != 0) {
				isError = true;
			}
		}
		message += string(buff, buff + readNum);
		state_machine();

		#ifdef TEST
		cout << "After state_machine!!" << endl;
		#endif

		//����ѭ��������
		if (isError == STATE_ERROR//��ʱ��ֱ�ӷ��أ��ͷ�
			|| isError == STATE_AGAIN//��ʱ����ı�״̬��Ȼ�����¼���
			|| isError == STATE_SUCCESS)//��ʱ����״̬�����������¼��룬������ͬ����ֱ���ͷ�
		{
			break;
		}
	}

	if (isError) {
		return -1;
	}
	if (state == FINISH) {
		if (keep_alive) {
			reset();//�̳�SingleTimer�����ش˺���
		}
		else
		{
			return -1;
		}
	}

	//���������ǰѸ��������ö�������ʱ���������
	push_to();

	//Ȼ��ע����¼�
	if (myEpoll == nullptr) {
		perror("myEpoll is nullptr!!!\n");
		return -1;
	}
	events = EPOLLIN | EPOLLET | EPOLLONESHOT;

	#ifdef TEST
		cout << "epoll->mod()" << endl;
	#endif // TEST

	myEpoll->mod(dynamic_pointer_cast<Task>(shared_from_this()));

}

ssize_t Task::readn(char* _buff) {
	size_t needToRead = BUFF_SIZE;
	ssize_t readOnce = 0;
	ssize_t readSum = 0;
	char* ptr = (char*) _buff
	#ifdef TEST
		cout << "readn" << endl;
	#endif
	while (needToRead > 0) {
		if ((readOnce == read(fd, ptr, needToRead)) < 0)
		{

			#ifdef TEST
				cout << "readOnce = " << readOnce << endl;
			#endif

			if(errno == EINTR || errno == EAGAIN)
			{
				readOnce = 0;
				continue;
			}
			else {
				return -1;
			}
		}
		readSum += readOnce;
		needToRead -= readOnce;
		ptr += readOnce;
	}
	return readSum;
}

ssize_t Task::writen(char* _buff, size_t _n) {
	size_t needToWrite = _n;
	ssize_t writeOnce = 0;
	ssize_t writeSum = 0;
	char* ptr = _buff;
	while (needToWrite > 0) {
		if ((writeOnce == write(fd, ptr, needToWrite)) < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				writeOnce = 0;
				continue;
			}
			else {
				return -1;
			}
		}
		writeSum += writeOnce;
		needToWrite -= writeOnce;
		ptr += writeOnce;
	}
	return writeSum;
}

void Task::state_machine() {
	switch (state) {
	case PARSE_URI:
		 parse_uri();
		 break;
	case PARSE_HEADERS:
		parse_headers();
		break;
	case RECV_BODY:
		recv_body();
		break;
	case ANALYSIS:
		analysis();
		break;
	}

}

void Task::parse_uri() {
	string& str = message;
	// �����������������ٿ�ʼ��������
	int pos = str.find('\r', readPos);
	if (pos < 0)
	{
		isError = STATE_AGAIN;
		return;
	}
	// ȥ����������ռ�Ŀռ䣬��ʡ�ռ�
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
		isError =  STATE_ERROR;
		return;
	}
	else
	{
		int _pos = requestLine.find(' ', pos);
		if (_pos < 0)
			return ;
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

	// HTTP �汾��
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
			isError =  STATE_ERROR;
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
				return ;
		}
	}
	state =PARSE_HEADERS;
	isError = STATE_SUCCESS;
	if (method == METHOD_POST) {
		state = RECV_BODY;
	}
	else {
		state = ANALYSIS;
	}
}

void Task::parse_headers() {
	string& str = message;
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
				if (key_end - key_start <= 0) {
					isError = STATE_ERROR;
					return;
				}
					
				h_state = H_COLON;
			}
			else if (str[i] == '\n' || str[i] == '\r') {
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
			else {
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
				if (value_end - value_start <= 0) {
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
			else {
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
				h_state =H_FINAL_N;
			}
			else {
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
		return ;
	}
	str = str.substr(now_read_line_begin);
	isError = STATE_AGAIN;

}

void Task::analysis() {

	if (method == METHOD_POST)
	{
	
		char header[BUFF_SIZE];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if (headers.find("Connection") != headers.end() && headers["Connection"] == "Keep-Alive")
		{
			keep_alive = true;
			sprintf(header, "%sConnection: Keep-Alive\r\n", header);
			sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
		}
		
		char* sendMessage = "I have receiced this.";

		sprintf(header, "%sContent-length: %zu\r\n", header, strlen(sendMessage));
		sprintf(header, "%s\r\n", header);
		size_t sendLen = (size_t)writen(header, strlen(header));
		if (sendLen != strlen(header))
		{
			perror("Send header failed");
			isError = true;
			return;
		}

		sendLen = (size_t)writen( sendMessage, strlen(sendMessage));
		if (sendLen != strlen(sendMessage))
		{
			perror("Send message failed");
			isError = true;
			return;
		}
		
		isError =  STATE_SUCCESS;
	}
	else if (method == METHOD_GET)
	{
		char header[BUFF_SIZE];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if (headers.find("Connection") != headers.end() && headers["Connection"] == "Keep-Alive")
		{
			keep_alive = true;
			sprintf(header, "%sConnection: Keep-Alive\r\n", header);
			sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
		}
		int dotPos = fileName.find('.');
		const char* filetype;
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
		// ͨ��Content-length�����ļ���С
		sprintf(header, "%sContent-length: %ld\r\n", header, sbuf.st_size);

		sprintf(header, "%s\r\n", header);
		size_t sendLen = (size_t)writen( header, strlen(header));
		if (sendLen != strlen(header))
		{
			perror("Send header failed");
			isError = STATE_ERROR;
			return;
		}
		int src_fd = open(fileName.c_str(), O_RDONLY, 0);
		char* src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		close(src_fd);

		// �����ļ���У��������
		sendLen = writen(src_addr, sbuf.st_size);
		if (sendLen != sbuf.st_size)
		{
			perror("Send file failed");
			isError = STATE_ERROR;
			return;
		}
		munmap(src_addr, sbuf.st_size);
		isError = STATE_SUCCESS;
	}
	else
	{
		isError = STATE_ERROR;
		return;
	}
}

void Task::recv_body() {
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
	if (message.size() < messageLength) {
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
	writen(send_buff, strlen(send_buff));
	sprintf(send_buff, "%s", body_buff.c_str());
	writen(send_buff, strlen(send_buff));
}