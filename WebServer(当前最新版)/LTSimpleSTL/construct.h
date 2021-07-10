//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//这个头文件用于构造与析构，负责的是在给定的内存区域上构造对象，和给定的内存区域上析构对象，
//但最重要的是，它不会回收或者申请内存。提供了五个内存管理工具中的两个。
//是第二个编写的头文件，

#include <new>
#include "type_traits.h"
#include "util.h"


namespace LT {
	//------------------------------------------------------construct-----------------------------------
	template<class T1>
	inline void construct(T1* _ptr1) {
		new(_ptr1) T1();//在_ptr1所指的内存空间创建一个类型为T1的对象
	}

	template<class T1, class T2>
	inline void construct(T1* _ptr1, const T2& _value) {
		new(_ptr1) T1(_value);
	}

	//右值引用的构造函数
	template <class T, class... Args>
	void construct(T* _ptr, Args&&... _args)
	{
		::new ((void*)_ptr) T(LT::forward<Args>(_args)...);
	}
	
	//--------------------------------------------------------destroy------------------------------------
	template<class T>
	inline void destroy(T* _ptr) {
		_ptr->~T();
	}

	template<class ForwardIterator>
	inline void destroy(ForwardIterator _first, ForwardIterator _last, true_type){}

	template<class ForwardIterator>
	inline void destroy(ForwardIterator _first, ForwardIterator _last, false_type) {
		while (_first != _last) {
			destroy(&*_first) ;
			++_first;
		}
	}

	template<class ForwardIterator>
	inline void destroy(ForwardIterator _first, ForwardIterator _last) {
		typedef typename _type_traits<ForwardIterator>::is_POD_type is_POD_type;
		destroy(_first, _last, is_POD_type());
	}
}