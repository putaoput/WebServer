// @Author Lin Tao
// @Email putaopu@qq.com

#pragma once
#include <pthread.h>
#include <unordered_map>
#include <string>

//MIME (Multipurpose Internet Mail Extensions) ��������Ϣ�������͵���������׼��
//MIME ��Ϣ�ܰ����ı���ͼ����Ƶ����Ƶ�Լ�����Ӧ�ó���ר�õ����ݡ�
class MimeType
{
private:
	static pthread_mutex_t lock;
	static std::unordered_map<std::string, std::string> mime;
	MimeType();
	MimeType(const MimeType& m);
public:
	//ͨ��һ����̬�Ĺ��г�Ա���������ǿ��Ե���˽�еĹ��캯����
	//�ŵ���ÿ�ε��øþ�̬���������ǿ��Լ����Ѿ����ڵĸö������
	static std::string getMime(const std::string& suffix);
};

