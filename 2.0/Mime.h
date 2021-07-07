// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once
#include <pthread.h>
#include <unordered_map>
#include <string>

//MIME (Multipurpose Internet Mail Extensions) 是描述消息内容类型的因特网标准。
//MIME 消息能包含文本、图像、音频、视频以及其他应用程序专用的数据。
class MimeType
{
private:
	static pthread_mutex_t lock;
	static std::unordered_map<std::string, std::string> mime;
	MimeType();
	MimeType(const MimeType& m);
public:
	//通过一个静态的公有成员函数，我们可以调用私有的构造函数，
	//优点是每次调用该静态函数，我们可以计数已经存在的该对象个数
	static std::string getMime(const std::string& suffix);
};

