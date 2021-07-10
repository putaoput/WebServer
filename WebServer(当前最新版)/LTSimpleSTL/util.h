//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
#include <cstddef>
#include "type_traits.h"
#include "iterator.h"
//该头文件用来实现一些琐碎的标准库功能.主要有
//move， forward，swap,iter_swap, 堆的辅助函数，pair及相关函数.

namespace LT {
	

	//----------------------------------右值----------------------------------------------
		//forward函数是c++11的新特性，也是一个标准库里的函数，
		//为了在使用右值引用参数的函数模板中解决参数的完美转发问题
		//std::forward<T>(u)有两个参数：T 与 u。当T为左值引用类型时，u将被转换为T类型的左值，否则u将被转换为T类型右值。
		//std::move是无条件的转为右值引用，而std::forward是有条件的转为右值引用
	template <class T>
	typename std::remove_reference<T>::type&& move(T&& _args) 
	{
		return static_cast<typename std::remove_reference<T>::type&&>(_args);
	}


	// forward

	template <class T>
	T&& forward(typename std::remove_reference<T>::type& _args) noexcept//告诉编译器不会抛出异常
	{
		return static_cast<T&&>(_args);
	}

	template <class T>
	T&& forward(typename std::remove_reference<T>::type&& _args) noexcept
	{
		//is_lvalue_reference：检查值是否为参考类型
		static_assert(!std::is_lvalue_reference<T>::value, "template argument"
			" substituting T is an lvalue reference type");
		return static_cast<T&&>(_args);
	}




	//-----------------------------------------swap-----------------------------------------------------
	//swap函数，这里用重要收获，原来c++里面是使用右值来加速交换的，减少了资源消耗

	template<class T>
	inline void swap(T& _lhs, T& _rhs) {
		T tmp = LT::move(_lhs);							//如果T&& tmp会移动失败，因为此时tmp此时仍然是一个右值引用。然后该右值会和左值a管理同一块内存
		_lhs = LT::move(_rhs);
		_rhs = LT::move(tmp);
	}
	//int型的swap使用位运算

	template<>
	inline void swap(int& _a, int& _b)
	{
		_a ^= _b;
		_b ^= _a;
		_a ^= _b;
	}

	template <class ForwardIter1, class ForwardIter2>
	inline ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 _last1, ForwardIter2 first2)
	{
		for (; first1 != _last1; ++first1, (void) ++first2)
			LT::swap(*first1, *first2);
		return first2;
	}

	//特例化，第二个参数可以输入个数
	template <class Tp, size_t _N>
	inline void swap(Tp(&_a)[_N], Tp(&_b)[_N])
	{
		LT::swap_range(_a, _a + _N, _b);
	}
	//迭代器的swap函数
	template <class ForwardIterator>
	inline void iter_swap(ForwardIterator _itA, ForwardIterator _itB) {
		LT::swap(*_itA, *_itB);
	}
	
	
	//--------------------------------pair----------------------------------------------------------
    //pair的泛型编程可能需要改进
    //std::pair<int, int> a;//这一行代码用来翻看源码
	template<class T1,class T2>
	struct pair
	{
		//从struct的泛型编程可以一窥泛型编程的思想
		typedef T1  first_type;
		typedef T2  second_type;//首先是要提供给使用者萃取成员变量类型的接口，

		T1 first;
		T2 second;
		//----------------------------------构造函数--------------------------------------

		pair(const T1& _first, const T2& _second):first(_first),second(_second){}
        pair(const pair<T1, T2>& _p) {
            first = _p.first;
            second = _p.second;
        }
		pair(const T1&& _first, const T2&& _second) {
			first = LT::move(_first);
			second = LT::move(_second);
		}
		pair& operator=(const pair& _p) {
			first = _p.first;
			second = _p.second;
			return *this;
		}

		pair& operator=(const pair&& _p) {
			first = LT::move(_p.first);//注意默认是在LT命名域下，所以调用的是LT
			second = LT::move(_p.second);
			return *this;
		}

		
		void swap(pair& _p) {
			LT::swap(_p.first, first);
			LT::swap(_p.second, second);
		}


	};

	//pair的一系列辅助函数：
	
	template<class T1, class T2>
	pair<T1, T2>  make_pair( T1&& _first,  T2&& _second) {
		return pair<T1, T2>(LT::forward<T1>(_first),LT::forward<T2>(_second));
	}
	
	template<class T1, class T2>
	pair<T1, T2>  make_pair(T1& _first, T2& _second) {
		return pair<T1, T2>((_first), (_second));
	}

	//重载以下操作符 ==, !=, >, >=, <,<=
	template<class T1, class T2>
	pair<T1, T2>  operator==(const pair<T1,T2>& _lhs, const pair<T1,T2>& _rhs) {
		return _lhs.first == _rhs.first && _lhs.second == _rhs.second;
	}

	template<class T1, class T2>
	pair<T1, T2>  operator!=(const pair<T1, T2>& _lhs, const pair<T1, T2>& _rhs) {
		return _lhs.first != _rhs.first || _lhs.second != _rhs.second;
	}

	template<class T1, class T2>
	pair<T1, T2>  operator > (const pair<T1, T2>& _lhs, const pair<T1, T2>& _rhs) {
		return _lhs.first > _rhs.first ||(_lhs.first == _rhs.first && _lhs.second > _rhs.second);
	}

	template<class T1, class T2>
	pair<T1, T2>  operator>=(const pair<T1, T2>& _lhs, const pair<T1, T2>& _rhs) {
		return !(_lhs < _rhs);
	}

	template<class T1, class T2>
	pair<T1, T2>  operator<(const pair<T1, T2>& _lhs, const pair<T1, T2>& _rhs) {
		return _lhs.first < _rhs.first && _lhs.second < _rhs.second;
	}

	template<class T1, class T2>
	pair<T1, T2>  operator<=(const pair<T1, T2>& _lhs, const pair<T1, T2>& _rhs) {
		return !(_lhs > _rhs);
	}
    

    // 重载 swap
    template <class T1, class T2>
    void swap(pair<T1, T2>& lhs, pair<T1, T2>& rhs)
    {
        lhs.swap(rhs);
    }

    
}
