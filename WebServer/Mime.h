// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once
#include <pthread.h>
#include <unordered_map>
#include <string>
//���õ���ģʽ��д��
//MIME (Multipurpose Internet Mail Extensions) ��������Ϣ�������͵���������׼��
//MIME ��Ϣ�ܰ����ı���ͼ����Ƶ����Ƶ�Լ�����Ӧ�ó���ר�õ����ݡ�
class MimeType
{
private:
	std::unordered_map<std::string, std::string> mime;
	MimeType(){}
	MimeType(  MimeType& _m);
	
public:
	//ͨ��һ����̬�Ĺ��г�Ա���������ǿ��Ե���˽�еĹ��캯����
	//�ŵ���ÿ�ε��øþ�̬���������ǿ��Լ����Ѿ����ڵĸö������
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