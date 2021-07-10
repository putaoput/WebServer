//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件套娃提供string类

#include "basic_string.h"

namespace LT {
	using string = LT::basic_string<char>; //C++编译器不支持使用typedef关键词为模板类设置别名
	using wstring = LT::basic_string<wchar_t>;
	using u16string = LT::basic_string<char16_t>;
	using u32string = LT::basic_string<char32_t>;
}

