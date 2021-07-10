//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ������ṩstring��

#include "basic_string.h"

namespace LT {
	using string = LT::basic_string<char>; //C++��������֧��ʹ��typedef�ؼ���Ϊģ�������ñ���
	using wstring = LT::basic_string<wchar_t>;
	using u16string = LT::basic_string<char16_t>;
	using u32string = LT::basic_string<char32_t>;
}

