// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once
#include <pthread.h>
#include <unordered_map>
#include <string>
//利用单例模式编写。
//MIME (Multipurpose Internet Mail Extensions) 是描述消息内容类型的因特网标准。
//MIME 消息能包含文本、图像、音频、视频以及其他应用程序专用的数据。
class MimeType
{
private:
	std::unordered_map<std::string, std::string> mime;
	MimeType(){}
	MimeType(  MimeType& _m);
	
public:
	//通过一个静态的公有成员函数，我们可以调用私有的构造函数，
	//优点是每次调用该静态函数，我们可以计数已经存在的该对象个数
	void init();
	std::string get_mime(std::string suffix);
	static MimeType* instance(){
		static MimeType m;
		return &m;
	}
};

class WebPage{
private:
	std::unordered_map<std::string, std::string> filesMap;
	WebPage(){}
	WebPage(  WebPage& _w);
public:
	void init();
	std::string get_page(std::string _fileName);
	static WebPage* instance(){
		static WebPage w;
		return &w;
	}
};