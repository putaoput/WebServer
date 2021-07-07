#include "requestData.h"
#include "util.h"
#include "epoll.h"
#include <sys/epoll.h>
#include <unistd.h> //��ͷ�ļ���c��c++�У��ṩ��POSIX����ϵͳAPI���ʹ��ܵ�ͷ�ļ�������
#include <sys/time.h>
#include <unordered_map>
#include <fcntl.h>
#include <sys/stat.h> //��unix/linuxϵͳ�����ļ�״̬���ڵ�α��׼ͷ�ļ���
#include <sys/mman.h>
#include <queue>
#include <string.h>


#include <iostream>
using namespace std;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
//��int pthread_mutex_init ֮��ĳ�ʼ��һ���������ķ�ʽ���ú꽫��������ÿ���ֶζ���ʼ��Ϊ��
std::unordered_map<std::string, std::string> MimeType::mime;
//����MimeType�е��ֵ�,��MimeType��������Ǿ�̬˽�е�

std::string MimeType::getMime(const std::string& suffix)
{
    //��һ�ε���ʱ��ʼ��mime�ֵ䣬����Ϊʲô����ǰ�涨���ʱ��ͳ�ʼ���أ�����Ϊ˽��û����ʼ����
    //���ǿ��Ե�������һ����ʼ�������ɣ���

    if (mime.size() == 0)
    {
        pthread_mutex_lock(&lock);
        if (mime.size() == 0)
        {
            mime[".html"] = "text/html";
            mime[".avi"] = "video/x-msvideo";
            mime[".bmp"] = "image/bmp";
            mime[".c"] = "text/plain";
            mime[".doc"] = "application/msword";
            mime[".gif"] = "image/gif";
            mime[".gz"] = "application/x-gzip";
            mime[".htm"] = "text/html";
            mime[".ico"] = "application/x-ico";
            mime[".jpg"] = "image/jpeg";
            mime[".png"] = "image/png";
            mime[".txt"] = "text/plain";
            mime[".mp3"] = "audio/mp3";
            mime["default"] = "text/html";
        }
        pthread_mutex_unlock(&lock);
    }

    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}

std::priority_queue<MyTimer*, std::deque<MyTimer*>, timer_cmp> myTimerQueue;

requestData::requestData() :
    now_read_pos(0), state(STATE_PARSE_URI), h_state(h_start),
    keep_alive(false), againTimes(0), timer(NULL)
{

}

requestData::requestData(int _epollfd, int _fd, std::string _path) :
    now_read_pos(0), state(STATE_PARSE_URI), h_state(h_start),
    keep_alive(false), againTimes(0), timer(NULL), path(_path), fd(_fd), epollfd(_epollfd)
{

}




//����requestData����Ĺ��캯��
requestData::~requestData()
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    //fd, epollfd��requestData�ڶ����һ��˽�г�Ա����
    //��request�ڲ����¼�����ɾ��
    if (timer != NULL)
    {
        timer->clearReq();
        timer = NULL;
    }
    close(fd);
}

void requestData::addTimer(MyTimer* mtimer)
{
    //���֮ǰû��
    if (timer == NULL)
    {
        timer = mtimer;
    }
}

//һЩά����װ�Եĺ���
int requestData::getFd()
{
    return fd;
}
void requestData::setFd(int _fd)
{
    fd = _fd;
}

void requestData::reset()
{
    againTimes = 0;
    content.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    keep_alive = false;
}

void requestData::seperateTimer()
{
    if (timer)
    {
        timer->clearReq();
        timer = NULL;
    }
}

//��������
void requestData::handleRequest()
{
   

    //���û�������״̬
    char buff[MAX_BUFF];
    bool isError = false;
    
    while (true)
    {
        //���Զ���Ķ�ȡ���ݵĺ������ж�ȡ�������Ƕ�ȡָ����n������
        int read_num = readn(fd, buff, MAX_BUFF);
        if (read_num < 0)
        {
            perror("readn error!!");
            isError = true;
            break;

        }
        else if (read_num == 0)
        {
            //��������֣����Ƕ��������ݣ�������Request Aborted�����������������û�дﵽ��ԭ��
            perror("read_num == 0");
            if (errno == EAGAIN)
            {
                if (againTimes > AGAIN_MAX_TIMES)
                    isError = true;
                else
                    ++againTimes;
            }
            else if (errno != 0)
                isError = true;
            break;
        }
	
	string now_read(buff, buff + read_num);
	content += now_read;
	cout << "content: " << endl;
	cout << content << endl;
	
	if (state == STATE_PARSE_URI)
        {
            int flag = this->parse_URI();
            if (flag == PARSE_URI_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                isError = true;
                break;
            }
        }
        if (state == STATE_PARSE_HEADERS)
        {
            int flag = this->parse_Headers();
            if (flag == PARSE_HEADER_AGAIN)
            {  
                break;
            }
            else if (flag == PARSE_HEADER_ERROR)
            {
                perror("3");
                isError = true;
                break;
            }
            if(method == METHOD_POST)
            {
                state = STATE_RECV_BODY;
            }
            else 
            {
                state = STATE_ANALYSIS;
            }
        }
        if (state == STATE_RECV_BODY)
        {
            int content_length = -1;
            if (headers.find("Content-length") != headers.end())
            {
                content_length = stoi(headers["Content-length"]);
            }
            else
            {
                isError = true;
                break;
            }
            if (content.size() < content_length)
                continue;
            state = STATE_ANALYSIS;
        }
        if (state == STATE_ANALYSIS)
        {
            int flag = this->analysisRequest();
            if (flag < 0)
            {
                isError = true;
                break;
            }
            else if (flag == ANALYSIS_SUCCESS)
            {

                state = STATE_FINISH;
                break;
            }
            else
            {
                isError = true;
                break;
            }
        }
   }
     
    //����ʧ���ˣ��������������
    if (isError)
    {
        cout << "isError" << endl;
        delete this;
        return;
    }

    // ����epoll����
    if (state == STATE_FINISH)
    {
        if (keep_alive)
        {
           // printf("ok\n");
            printf("long connection!");
            this->reset();
        }
        else
        {
            printf("short connection");
            delete this;
            return;
        }
    }

    //qlock��main�����ﶨ���һ������

    //������һ��ʱ����Ϣ
    pthread_mutex_lock(&qlock);
    MyTimer* mtimer = new MyTimer(this, 500);
    timer = mtimer;
    myTimerQueue.push(mtimer);
    pthread_mutex_unlock(&qlock);

    __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_mod(epollfd, fd, static_cast<void*>(this), _epo_event);
    //�޸��¼���ʾ��
    if (ret < 0)
    {
        //�޸Ĵ����򷵻�
        delete this;
        return;
    }

    /*
    ����ETģʽ�£����readһ��û�ж�����buffer�е����ݣ���ô�´ν��ò�����������֪ͨ�����buffer�����е������޻���������������µ������ٴε������д��������Ҫ����ΪETģʽ��fdͨ��Ϊ��������ɵ�һ�����⡪����α�֤���û�Ҫ��д������д�ꡣ
    Ҫ�����������ETģʽ�µĶ�д���⣬���Ǳ���ʵ�֣�
        a. ���ڶ���ֻҪbuffer�л������ݾ�һֱ����
        b. ����д��ֻҪbuffer���пռ����û�����д�����ݻ�δд�꣬��һֱд��
    */

}

//��URI�����л�ȡ��Ҫ����Ϣ

//ע������������Ľ����ȥ�����Ѿ�������������

int requestData::parse_URI()
{
    std::string& str = content;
    //content ���ݶ�����requestData������������Ƕ�����һ���Ըö��������
    int pos = str.find('\r',now_read_pos);
    //�ӵ�ǰ��ȡλ�ÿ�ʼ��Ѱ�һ��з���ֱ����ȡ�����з����ٿ�ʼ��������pos���ص��ǻ��з�λ��
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    //ȥ����������ռ�ռ䣬��Լ�ռ�
    std::string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
        str = str.substr(pos + 1);
    else
        str.clear();
    
    //�ٷ������õķ���
    //ͨ������������Ѱ��GET��POST�ַ����ķ������������÷���
    pos = request_line.find("GET");

    if (pos < 0)
    {
        pos = request_line.find("POST");
        if (pos < 0)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            method = METHOD_POST;
        }
    }
    else
    {
        method = METHOD_GET;
    }


    //�����������󣬻�ȡ�ļ���
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;//�����յ��˲������������ģ������URI����
    }
    else {
        //����������Ѱ����һ���ո���ֵ�λ��
        int _pos = request_line.find(' ', pos);
        if (_pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if (_pos - pos > 1)
            {
                file_name = request_line.substr(pos + 1, _pos - pos - 1);
                int __pos = file_name.find('?');
                if (__pos >= 0)
                {
                    file_name = file_name.substr(0, __pos);
                }
            }
            else
                file_name = "index.html";
        }
        pos = _pos;
    }


    //������Ѱ�� ,��ȡ HTTP �汾��
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        if (request_line.size() - pos <= 3)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            std::string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0")
                HTTPversion = HTTP_10;
            else if (ver == "1.1")
                HTTPversion = HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    state = STATE_PARSE_HEADERS;

    //����Ҫ�ض���ȡ�ɹ�֮�󣬷���URI�����ɹ��ı�ʶ
    return PARSE_URI_SUCCESS;
}
   
//ע������������Ľ����ȥ�����Ѿ�������������
//���������ͷ��
int requestData::parse_Headers()
{

    std::string& str = content;

    //����һЩ�������ϵĲ�������־��������
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;

    for (int i = 0; i < str.size() && notFinish; ++i)
    {
        switch (h_state)//������requestData�ṹ���е�˽�г�Ա����,����ʱΪh_start
            /*
            * enum HeadersState
            {
    //////////h_start = 0,
    //////////h_key,
    //////////h_colon,
    //////////h_spaces_after_colon,
    //////////h_value,
    //////////h_CR,
    //////////h_LF,
    //////////h_end_CR,
    //////////h_end_LF
                };
            */
        {
        case h_start:
        {
            if (str[i] == '\n' || str[i] == '\r')//�����˻��з�
                break;
            h_state = h_key;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        case h_key:
        {
            if (str[i] == ':')
            {
                key_end = i;
                if (key_end - key_start <= 0)
                    return PARSE_HEADER_ERROR;
                h_state = h_colon;
            }
            else if (str[i] == '\n' || str[i] == '\r')
                return PARSE_HEADER_ERROR;
            break;
        }
        case h_colon:
        {
            if (str[i] == ' ')
            {
                h_state = h_spaces_after_colon;
            }
            else
                return PARSE_HEADER_ERROR;
            break;
        }
        case h_spaces_after_colon:
        {
            h_state = h_value;
            value_start = i;
            break;
        }
        case h_value:
        {
            if (str[i] == '\r')
            {
                h_state = h_CR;
                value_end = i;
                if (value_end - value_start <= 0)
                    return PARSE_HEADER_ERROR;
            }
            else if (i - value_start > 255)
                return PARSE_HEADER_ERROR;
            break;
        }
        case h_CR:
        {
            if (str[i] == '\n')
            {
                h_state = h_LF;
                std::string key(str.begin() + key_start, str.begin() + key_end);
                std::string value(str.begin() + value_start, str.begin() + value_end);

                //test:
                cout << "key = " << key << endl;
                cout << "value = " << value << endl;

                headers[key] = value;
                now_read_line_begin = i;
            }
            else
                return PARSE_HEADER_ERROR;
            break;
        }
        case h_LF:
        {
            if (str[i] == '\r')
            {
                h_state = h_end_CR;
            }
            else
            {
                key_start = i;
                h_state = h_key;
            }
            break;
        }
        case h_end_CR:
        {
            if (str[i] == '\n')
            {
                h_state = h_end_LF;
            }
            else
                return PARSE_HEADER_ERROR;
            break;
        }
        case h_end_LF:
        {
            notFinish = false;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        }
    }

    //�ɹ�
    if (h_state == h_end_LF)
    {
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }
    //ʧ�ܣ�����״̬����һ��
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}

int requestData::analysisRequest()
{
    if (method == METHOD_POST)
    {
        //get content
        char header[MAX_BUFF];
        sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        if (headers.find("Connection") != headers.end() && headers["Connection"] == "Keep-Alive")
        {
            keep_alive = true;
            sprintf(header, "%sConnection: Keep-Alive\r\n", header);
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
        }
        //cout << "content=" << content << endl;
        // test char*
        char* send_content = "I have receiced this.";

        sprintf(header, "%sContent-length: %zu\r\n", header, strlen(send_content));
        sprintf(header, "%s\r\n", header);
        size_t send_len = (size_t)writen(fd, header, strlen(header));
        if (send_len != strlen(header))
        {
            perror("Send header failed");
            return ANALYSIS_ERROR;
        }

        send_len = (size_t)writen(fd, send_content, strlen(send_content));
        if (send_len != strlen(send_content))
        {
            perror("Send content failed");
            return ANALYSIS_ERROR;
        }
        std::cout << "content size ==" << content.size() << std::endl;
   

        return ANALYSIS_SUCCESS;
    }
    else if (method == METHOD_GET)
    {
        char header[MAX_BUFF];
        sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        if (headers.find("Connection") != headers.end() && headers["Connection"] == "Keep-Alive")
        {
            cout << "Keep-Alive" << endl;
            keep_alive = true;
            sprintf(header, "%sConnection: keep-alive\r\n", header);
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
        }
        int dot_pos = file_name.find('.');
        const char* filetype;
        if (dot_pos < 0)
            filetype = MimeType::getMime("default").c_str();
        else
            filetype = MimeType::getMime(file_name.substr(dot_pos)).c_str();
        struct stat sbuf;
        if (stat(file_name.c_str(), &sbuf) < 0)
        {
            handleError(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }

        sprintf(header, "%sContent-type: %s\r\n", header, filetype);
        // ͨ��Content-length�����ļ���С
        sprintf(header, "%sContent-length: %ld\r\n", header, sbuf.st_size);

        sprintf(header, "%s\r\n", header);
        size_t send_len = (size_t)writen(fd, header, strlen(header));
        if (send_len != strlen(header))
        {
            perror("Send header failed");
            return ANALYSIS_ERROR;
        }
        int src_fd = open(file_name.c_str(), O_RDONLY, 0);
        char* src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        //void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
        //��һ���ļ�������������ӡ����ڴ�
        //��LINUX�����ǿ���ʹ��mmap�����ڽ��������ڴ��ַ�ռ��з����ַ�ռ䣬�����������ڴ��ӳ���ϵ��


        close(src_fd);

        // �����ļ���У��������
        send_len = writen(fd, src_addr, sbuf.st_size);
        if (send_len != sbuf.st_size)
        {
            perror("Send file failed");
            return ANALYSIS_ERROR;
        }
        munmap(src_addr, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    else
        return ANALYSIS_ERROR;
}

void requestData::handleError(int fd, int err_num, std::string short_msg)
{
    short_msg = " " + short_msg;
    char send_buff[MAX_BUFF];
    std::string body_buff, header_buff;
    body_buff += "<html><title>TKeed Error</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += std::to_string(err_num) + short_msg;
    body_buff += "<hr><em> Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-type: text/html\r\n";
    header_buff += "Connection: close\r\n";
    header_buff += "Content-length: " + std::to_string(body_buff.size()) + "\r\n";
    header_buff += "\r\n";
    sprintf(send_buff, "%s", header_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
}

//��ʱ����Ĺ��캯��
MyTimer::MyTimer(requestData* _request_data, int timeout) :deleted(false), request_data(_request_data)
{
    struct timeval now;
    gettimeofday(&now, NULL);//ֱ�ӻ��ʱ��
    /*
    //*struct timeval {
    //*    long    tv_sec;         /* seconds /
    //*long    tv_usec;        /* and microseconds /us
    //};
    */
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

MyTimer::~MyTimer()
{
    if (request_data != NULL)
    {
        //ע�������б������������Ұָ���������
        //�����֣���Ա��������һ��ָ����࣬Ҫ�Լ�д����������������Դ�ͷŲ���ȫ
        delete request_data;
        request_data = NULL;
    }
}

//���¼�ʱ��
void MyTimer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

//�жϼ�ʱ���Ƿ���Ч
//�Ѿ���ʱ����Ч�����Ѹü�ʱ����ɾ����־��Ϊtrue
bool MyTimer::isvalid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    if (temp < expired_time)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

//
void MyTimer::clearReq()
{
    request_data = NULL;
    this->setDeleted();
}

void MyTimer::setDeleted()
{
    deleted = true;
}

bool MyTimer::isDeleted() const
{
    return deleted;
}

size_t MyTimer::getExpTime() const
{
    return expired_time;
}

bool timer_cmp::operator()(const MyTimer* a, const MyTimer* b) const
{
    return a->getExpTime() > b->getExpTime();
}

