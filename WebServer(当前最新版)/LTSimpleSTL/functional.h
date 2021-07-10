//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件提供仿函数
#include <cstddef>
#include "util.h"
namespace LT {
	//定义一元函数的参数型别和返回值型别
	template<class Arg, class Result>
	struct single_function {
		typedef Arg argument_type;
		typedef Result result_type;
	};

	//定义二元函数的参数型别和返回值型别
	template<class Arg1, class Arg2, class Result>
	struct binary_function {
		typedef Arg1 _first_argument_type;
		typedef Arg2 second_argument_type;
		typedef Result result_type;
	};

	//----------------------------------------算术运算仿函数-------------------------------
	template<class T>
	struct plus:public binary_function<T,T,T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 + _arg2;
		}
	};

	template<class T>
	struct minus :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 - _arg2;
		}
	};

	template<class T>
	struct multiplies :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 * _arg2;
		}
	};

	template<class T>
	struct divides :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 / _arg2;
		}
	};

	template<class T>
	struct modulus :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 % _arg2;
		}
	};

	//----------------------------------------比较运算符----------------------------------
	template<class T>
	struct equal_to:public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 == _arg2;
		}
	};

	template<class T>
	struct not_equal_to :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 != _arg2;
		}
	};

	template<class T>
	struct greater :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 > _arg2;
		}
	};

	template<class T>
	struct less :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 < _arg2;
		}
	};

	template<class T>
	struct greater_and_equal :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 >= _arg2;
		}
	};

	template<class T>
	struct less_and_equal :public binary_function<T, T, T>
	{
		T operator()(const T& _arg1, const T& _arg2) {
			return _arg1 <= _arg2;
		}
	};

	//取负数
	template <class T>
	struct negate :public single_function<T, T>
	{
		T operator()(const T& _arg) const { return -_arg; }
	};

	//---------------------------------逻辑运算符---------------------------------------------
	// 函数对象：逻辑与
	template <class T>
	struct logical_and :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x && y; }
	};

	// 函数对象：逻辑或
	template <class T>
	struct logical_or :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x || y; }
	};

	// 函数对象：逻辑非
	template <class T>
	struct logical_not :public single_function<T, bool>
	{
		bool operator()(const T& x) const { return !x; }
	};

	//-----------------------------------------证同-----------------------------------------------
	//如果元素类型不同，将出现编译期错误

	// 加法的证同元素
	template <class T>
	T identity_element(plus<T>) { return T(0); }

	// 乘法的证同元素
	template <class T>
	T identity_element(multiplies<T>) { return T(1); }

	// 证同函数：不会改变元素，返回本身
	template <class T>
	struct identity :public single_function<T, bool>
	{
		const T& operator()(const T& _arg) const { return _arg; }
	};

	//------------------------------------------投射与选择函数-------------------------------------------

	// 选择函数：接受一个 pair，返回第一个元素
	template <class Pair>
	struct get_first :public single_function<Pair, typename Pair::_first_type>
	{
		const typename Pair::_first_type& operator()(const Pair& _arg) const
		{
			return _arg._first;
		}
	};

	// 选择函数：接受一个 pair，返回第二个元素
	template <class Pair>
	struct get_second :public single_function<Pair, typename Pair::_second_type>
	{
		const typename Pair::second_type& operator()(const Pair& _arg) const
		{
			return _arg.second;
		}
	};

	// 投射函数：返回第一参数
	template <class Arg1, class Arg2>
	struct project_first :public binary_function<Arg1, Arg2, Arg1>
	{
		Arg1 operator()(const Arg1& _arg, const Arg2&) const { return _arg; }
	};

	// 投射函数：返回第二参数
	template <class Arg1, class Arg2>
	struct project_second :public binary_function<Arg1, Arg2, Arg2>
	{
		Arg2 operator()(const Arg1&, const Arg2& _arg) const { return _arg; }
	};


	//-------------------------------------------------哈希函数--------------------------------------------------------------------
	/*****************************************************************************************/
// 哈希函数对象

	template <class Key>
	struct hash {};

	
	// 针对指针的偏特化版本
	template <class T>
	struct hash<T*>
	{
		size_t operator()(T* _p) const noexcept
		{
			return reinterpret_cast<size_t>(_p);
		}
	};

	//整数类型
	// 对于整型类型，只是返回原值
	template<>
	struct hash<short> {
		size_t operator()(short _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<unsigned short> {
		size_t operator()(unsigned short _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<int> {
		size_t operator()(int _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<unsigned int> {
		size_t operator()(unsigned int _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<long> {
		size_t operator()(long _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<unsigned long> {
		size_t operator()(unsigned long _val) {
			return static_cast<size_t>(_val);
		}
	};


	template<>
	struct hash<unsigned long long> {
		size_t operator()(unsigned long long _val) {
			return static_cast<size_t>(_val);
		}
	};

	

	template<>
	struct hash<char> {
		size_t operator()(char _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<signed char> {
		size_t operator()(signed _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<unsigned char> {
		size_t operator()(unsigned char _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<wchar_t> {
		size_t operator()(wchar_t _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<char16_t> {
		size_t operator()(char16_t _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<char32_t> {
		size_t operator()(char32_t _val) {
			return static_cast<size_t>(_val);
		}
	};

	template<>
	struct hash<bool> {
		size_t operator()(bool _val) {
			return static_cast<size_t>(_val);
		}
	};

		// 对于浮点数，逐位哈希
		inline size_t bitwise_hash(const unsigned char* _first, size_t _count)
	{
#if (_MSC_VER && _WIN64) || ((__GNUC__ || __clang__) &&__SIZEOF_POINTER__ == 8)//不同编译器不同的设置
		const size_t fnv_offset = 14695981039346656037ull;
		const size_t fnv_prime = 1099511628211ull;
#else
		const size_t fnv_offset = 2166136261u;
		const size_t fnv_prime = 16777619u;
#endif
		size_t result = fnv_offset;
		for (size_t i = 0; i < _count; ++i)
		{
			result ^= (size_t)_first[i];
			result *= fnv_prime;
		}
		return result;
	}

	template <>
	struct hash<float>
	{
		size_t operator()(const float& _val)
		{
			return _val == 0.0f ? 0 : bitwise_hash((const unsigned char*)&_val, sizeof(float));
		}
	};

	template <>
	struct hash<double>
	{
		size_t operator()(const double& _val)
		{
			return _val == 0.0f ? 0 : bitwise_hash((const unsigned char*)&_val, sizeof(double));
		}
	};

	template <>
	struct hash<long double>
	{
		size_t operator()(const long double& val)
		{
			return val == 0.0f ? 0 : bitwise_hash((const unsigned char*)&val, sizeof(long double));
		}
	};

}