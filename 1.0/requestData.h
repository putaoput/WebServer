//请求数据头文件中定义了时间堆MyTimer用来处理超时时间，封装好的requestData对象
//每个request对象会包含一个指向时间堆的指针

#ifndef REQUESTDATA
#define REQUESTDATA

#include<string>
#include<unordered_map>

// parse:解析
constexpr int STATE_PARSE_URI = 1;
constexpr int STATE_PARSE_HEADERS = 2;
constexpr int STATE_RECV_BODY = 3;
constexpr int STATE_ANALYSIS = 4;
constexpr int STATE_FINISH = 5;

constexpr int MAX_BUFF = 4096;

// 有请求出现但是读不到数据,可能是Request Aborted,
// 或者来自网络的数据没有达到等原因,
// 对这样的请求尝试超过一定的次数就抛弃
constexpr int AGAIN_MAX_TIMES = 200;

constexpr int PARSE_URI_AGAIN = -1;
constexpr int PARSE_URI_ERROR = -2;
constexpr int PARSE_URI_SUCCESS = 0;

constexpr int PARSE_HEADER_AGAIN = -1;
constexpr int PARSE_HEADER_ERROR = -2;
constexpr int PARSE_HEADER_SUCCESS = 0;

constexpr int ANALYSIS_ERROR = -2;
constexpr int ANALYSIS_SUCCESS = 0;

constexpr int METHOD_POST = 1;
constexpr int METHOD_GET = 2;
constexpr int HTTP_10 = 1;
constexpr int HTTP_11 = 2;

constexpr int EPOLL_WAIT_TIME = 500;

//MIME (Multipurpose Internet Mail Extensions) 是描述消息内容类型的因特网标准。
//MIME 消息能包含文本、图像、音频、视频以及其他应用程序专用的数据。
class MimeType
{
private:
	static pthread_mutex_t lock;
	static std::unordered_map<std::string, std::string> mime;
	MimeType();
	MimeType(const MimeType &m);
public:
	//通过一个静态的公有成员函数，我们可以调用私有的构造函数，
	//优点是每次调用该静态函数，我们可以计数已经存在的该对象个数
	static std::string getMime(const std::string& suffix);
};

enum HeadersState
{
	h_start = 0,
	h_key,
	h_colon,
	h_spaces_after_colon,
	h_value,
	h_CR,
	h_LF,
	h_end_CR,
	h_end_LF
};

struct MyTimer;
struct requestData;

struct requestData
{
private:
    int againTimes;
    std::string path;
    int fd;
    int epollfd;
    // content的内容用完就清
    std::string content;
    int method; //post或是get
    int HTTPversion;
    std::string file_name;
    int now_read_pos;
    int state;
    int h_state;
    bool isfinish;
    bool keep_alive; //标识长连接还是短连接
    std::unordered_map<std::string, std::string> headers;
    MyTimer* timer;

 
private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();

public:

    requestData();
    requestData(int _epollfd, int _fd, std::string _path);
    ~requestData();
    void addTimer(MyTimer* mtimer);
    void reset();
    void seperateTimer();
    int getFd();
    void setFd(int _fd);
    void handleRequest();
    void handleError(int fd, int err_num, std::string short_msg);
};

struct MyTimer
{
    bool deleted;
    size_t expired_time;
    requestData* request_data;

    MyTimer(requestData* _request_data, int timeout);
    ~MyTimer();
    void update(int timeout);
    bool isvalid();
    void clearReq();
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;
};

struct timer_cmp
{
    bool operator()(const MyTimer* a, const MyTimer* b) const;
};
#endif // !REQUESTDATA
