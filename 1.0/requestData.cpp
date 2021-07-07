#include "requestData.h"
#include "util.h"
#include "epoll.h"
#include <sys/epoll.h>
#include <unistd.h> //该头文件是c和c++中，提供对POSIX操作系统API访问功能的头文件的名称
#include <sys/time.h>
#include <unordered_map>
#include <fcntl.h>
#include <sys/stat.h> //是unix/linux系统定义文件状态所在的伪标准头文件。
#include <sys/mman.h>
#include <queue>
#include <string.h>


#include <iostream>
using namespace std;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
//除int pthread_mutex_init 之外的初始化一个互斥锁的方式，该宏将互斥锁的每个字段都初始化为零
std::unordered_map<std::string, std::string> MimeType::mime;
//声明MimeType中的字典,在MimeType里面这个是静态私有的

std::string MimeType::getMime(const std::string& suffix)
{
    //第一次调用时初始化mime字典，但是为什么不在前面定义的时候就初始化呢，是因为私有没法初始化吗？
    //可是可以单独定义一个初始化函数吧？？

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




//定义requestData对象的构造函数
requestData::~requestData()
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    //fd, epollfd是requestData内定义的一个私有成员变量
    //从request内部的事件表中删除
    if (timer != NULL)
    {
        timer->clearReq();
        timer = NULL;
    }
    close(fd);
}

void requestData::addTimer(MyTimer* mtimer)
{
    //如果之前没有
    if (timer == NULL)
    {
        timer = mtimer;
    }
}

//一些维护封装性的函数
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

//处理请求
void requestData::handleRequest()
{
   

    //设置缓冲区和状态
    char buff[MAX_BUFF];
    bool isError = false;
    
    while (true)
    {
        //用自定义的读取数据的函数进行读取，功能是读取指定的n个数据
        int read_num = readn(fd, buff, MAX_BUFF);
        if (read_num < 0)
        {
            perror("readn error!!");
            isError = true;
            break;

        }
        else if (read_num == 0)
        {
            //有请求出现，但是读不到数据，可能是Request Aborted，或者是网络的数据没有达到等原因
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
     
    //请求失败了，析构掉这个请求
    if (isError)
    {
        cout << "isError" << endl;
        delete this;
        return;
    }

    // 加入epoll继续
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

    //qlock是main函数里定义的一个锁、

    //先增加一个时间信息
    pthread_mutex_lock(&qlock);
    MyTimer* mtimer = new MyTimer(this, 500);
    timer = mtimer;
    myTimerQueue.push(mtimer);
    pthread_mutex_unlock(&qlock);

    __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_mod(epollfd, fd, static_cast<void*>(this), _epo_event);
    //修改事件提示符
    if (ret < 0)
    {
        //修改错误则返回
        delete this;
        return;
    }

    /*
    对于ET模式下，如果read一次没有读尽尽buffer中的数据，那么下次将得不到读就绪的通知，造成buffer中已有的数据无机会读出，除非有新的数据再次到达。对于写操作，主要是因为ET模式下fd通常为非阻塞造成的一个问题——如何保证将用户要求写的数据写完。
    要解决上述两个ET模式下的读写问题，我们必须实现：
        a. 对于读，只要buffer中还有数据就一直读；
        b. 对于写，只要buffer还有空间且用户请求写的数据还未写完，就一直写。
    */

}

//从URI请求中获取想要的信息

//注意在这里分析的结果是去掉了已经被分析的内容

int requestData::parse_URI()
{
    std::string& str = content;
    //content 内容定义在requestData对象里，这里面是定义了一个对该对象的引用
    int pos = str.find('\r',now_read_pos);
    //从当前读取位置开始，寻找换行符，直到读取到换行符，再开始解析请求，pos返回的是换行符位置
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    //去掉请求行所占空间，节约空间
    std::string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
        str = str.substr(pos + 1);
    else
        str.clear();
    
    //①分析需用的方法
    //通过在请求行中寻找GET或POST字符串的方法来区分所用方法
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


    //继续分析请求，获取文件名
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;//代表收到了不能正常解析的，错误的URI请求
    }
    else {
        //接下来继续寻找下一个空格出现的位置
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


    //第三步寻找 ,获取 HTTP 版本号
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

    //三个要素都获取成功之后，返回URI分析成功的标识
    return PARSE_URI_SUCCESS;
}
   
//注意在这里分析的结果是去掉了已经被分析的内容
//分析请求的头部
int requestData::parse_Headers()
{

    std::string& str = content;

    //定义一些将被用上的参数来标志分析过程
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;

    for (int i = 0; i < str.size() && notFinish; ++i)
    {
        switch (h_state)//定义在requestData结构体中的私有成员变量,构造时为h_start
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
            if (str[i] == '\n' || str[i] == '\r')//读到了换行符
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

    //成功
    if (h_state == h_end_LF)
    {
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }
    //失败，返回状态再试一次
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
        // 通过Content-length返回文件大小
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
        //将一个文件或者其他对象印射进内存
        //在LINUX中我们可以使用mmap用来在进程虚拟内存地址空间中分配地址空间，创建和物理内存的映射关系。


        close(src_fd);

        // 发送文件并校验完整性
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

//计时器类的构造函数
MyTimer::MyTimer(requestData* _request_data, int timeout) :deleted(false), request_data(_request_data)
{
    struct timeval now;
    gettimeofday(&now, NULL);//直接获得时间
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
        //注意这里有避免产生空悬的野指针操作！！
        //像这种，成员变量里有一个指针的类，要自己写析构函数，避免资源释放不完全
        delete request_data;
        request_data = NULL;
    }
}

//更新计时器
void MyTimer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

//判断计时器是否有效
//已经超时则无效，并把该计时器的删除标志设为true
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

