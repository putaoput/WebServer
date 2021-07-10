//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件负责实现uninitialized_copy(),uninitialized_fill_n(),uninitialized_fiil()三个内存管理函数
//实现了支持右值引用的uninitialized_move()
//由于存在对非基本数据类型的加速处理，所以一定要保证输入的一对Output Interator迭代器对应与一块连续内存
//上述三个低层次的函数对应三个高层次函数:copy(),fill(),fill_n()。
//实际上是需要异常处理的
//要求具有回滚功能，即commit or rollback。要么全部构造，要么一个也不构造
//对于POD类型采用最高效得方式，对于非POD类型采用最保险的方式
#include "construct.h"
#include "memory.h"
#include "iterator.h"
#include "type_traits.h"
#include "algobase.h" //这里有更高阶的内存分配函数，用来管理基本数据类型的内存
#include "util.h"
#include <iostream>
#


namespace LT {
	
	//-------------------------------------------uninitialized_fiil_n()的实现---------------------------------------------
	
	//true_type被派送至的模板函数
	template <typename ForwardIterator, class Size, class T>
	inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator _first, Size _n, const T& _value, true_type) {
		return LT::fill_n(_first, _n, _value);//交给更高阶的函数执行
	}
	//__flase_type被派送至的模板函数
	template <typename ForwardIterator, class Size, class T>
	inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator _first, Size _n, const T& _value, false_type) {
		ForwardIterator cur = _first;	
		try {
			for (; _n > 0; --_n, ++cur) {
				construct(&*cur, _value);
			}
			return cur;
		}
		catch(...){
			//此处应当有异常处理,以保证commit or rollback。
			for (auto it = _first; cur != it; ++ it) {
				destroy(&*it);
			}
			return _first;
		}	
	}

	template <typename ForwardIterator, class Size, class T, class Val_t>
	inline ForwardIterator __uninitialized_fill_n(ForwardIterator _first, Size _n, const T& _value, Val_t*) {
		typedef typename _type_traits<T>::is_POD_type is_POD;//这一步会萃取出是否是基本数据类型，结果是struct true_type 或者false_type
		return __uninitialized_fill_n_aux(_first, _n, _value, is_POD());
		//这个会根据true_type 或者 false_type 派送到不同的模板函数。
	}


	//该函数的进行逻辑是首先萃取出迭代器的value_type，然后判断该value_type是否是POD型别。
	template <typename ForwardIterator, class Size, class T>
	inline ForwardIterator uninitialized_fill_n(ForwardIterator _first, Size _n, const T& _value) {
		return __uninitialized_fill_n(_first, _n, _value, value_type(_first));
		//用来萃取类型，value_type萃取出来的其实是该类型的指针。
	}


	//-------------------------------------------uninitialized_copy()的实现---------------------------------------------
	template <typename InputIterator, class ForwardIterator>//基本数据类型
	inline ForwardIterator __uninitialized_copy_aux(InputIterator _first, InputIterator _last, ForwardIterator _result, true_type) {
		return LT::copy(_first, _last, _result);//交由高阶函数处理
	}

	template <typename InputIterator, class ForwardIterator>//非基本数据类型
	inline ForwardIterator __uninitialized_copy_aux(InputIterator _first, InputIterator _last, ForwardIterator _result, false_type) {
		InputIterator copyStart = _first;
		ForwardIterator cur = _result;
		int n = _last - _first;
		try {
			for (; n > 0; --n, ++cur) {
				construct(&*cur, *copyStart++);
			}
			return cur;
		}
		catch (...) {
			//此处应当有异常处理,以保证commit or rollback。
			for (auto it = _result; cur != it; ++it) {
				destroy(&*it);
			}
			return _result;
		}
	}

	//要对char* 和 wchar_t*提供更为高效的特化版本

	inline char* uninitialized_copy(const char* _first, const char* _last, char* _result) {
		std::memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}

	inline wchar_t* uninitialized_copy(const wchar_t* _first, const wchar_t* _last, wchar_t* _result) {
		memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}

	template <typename InputIterator, class ForwardIterator, class T>//分辨是否是基本数据类型
	inline ForwardIterator __uninitialized_copy(InputIterator _first, InputIterator _last, ForwardIterator _result, T*) {
		typedef typename _type_traits<T>::is_POD_type is_POD;
		return __uninitialized_copy_aux(_first, _last, _result, is_POD());
	}

	

	//首先是提供一组拷贝函数，负责把一个区域的内容拷贝到另一个区域,返回拷贝结束的位置的迭代器
	template <typename InputIterator, class ForwardIterator>//基本数据类型
	inline ForwardIterator uninitialized_copy(InputIterator _first, InputIterator _last, ForwardIterator _result) 
	{
		return __uninitialized_copy(_first, _last, _result, value_type(_result));//萃取value_type
	}


	//-------------------------------------------uninitialized_fill()的实现---------------------------------------------

	template <typename ForwardIterator, class T>
	inline void __uninitialized_fill_aux(ForwardIterator _first, ForwardIterator _last, const T& _value, true_type) {
		fill(_first, _last, _value);//调用stl算法fill().
	}

	template <typename ForwardIterator, class T>
	inline void __uninitialized_fill_aux(ForwardIterator _first, ForwardIterator _last, const T& _value, false_type) {
		ForwardIterator cur = _first;
		try {
			for (; cur != _last; ++cur) {
				construct(&*cur, _value);
			}
			return cur;
		}
		catch(...){
			//失败了要进行析构
			for (auto it = _first; cur != it; ++it) {
				destroy(&*it);
			}
			return _first;
		}
	}

	template <typename ForwardIterator, class T, class T1>
	inline void __uninitialized_fill(ForwardIterator _first, ForwardIterator _last, const T& _value, T1*) {
		typedef typename _type_traits<T>::is_POD_type is_POD;
		__uninitialized_fill_aux(_first, _last, _value, is_POD());
	}

	template <typename ForwardIterator, class T>
	void uninitialized_fill(ForwardIterator _first, ForwardIterator _last, const T& _value) {
		__uninitialized_fill(_first, _last, _value, value_type(_first));
	}
	//------------------------------------------------------uninitialized_move----------------------------------------------------
	template <typename InputIterator, class ForwardIterator>
	ForwardIterator __uninitialized_move(InputIterator _first, InputIterator _last, ForwardIterator _result, std::true_type)
	{
		return LT::move(_first, _last, _result);
	}

	template <typename InputIterator, class ForwardIterator>
	ForwardIterator __uninitialized_move(InputIterator _first, InputIterator _last, ForwardIterator _result, std::false_type)
	{
		ForwardIterator cur = _result;
		try
		{
			size_t n = LT::distance(_first, _last);
			for (; n; ++_first, ++cur, --n)
			{
				LT::construct(LT::address_of(*cur), LT::move(*_first));
			}
		}
		catch (...)
		{
			LT::destroy(_result, cur);
		}
		return cur;
	}

	template <typename InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_move(InputIterator _first, InputIterator _last, ForwardIterator _result)
	{
		return __uninitialized_move(_first, _last, _result,
			std::is_trivially_move_assignable<typename iterator_traits<InputIterator>::value_type>{});
	}
}


