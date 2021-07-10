//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件是空间配置器之一，简单的对new 和delete进行了封装。不作为默认空间配置器
//只分配内存
#include "construct.h"

namespace LT {
	//
	template <class T>
	class allocator
	{
	public://用来提取类型
		typedef T            value_type;
		typedef T*           pointer;
		typedef const T*     const_pointer;
		typedef T&           reference;
		typedef const T&     const_reference;
		typedef size_t       size_type;
		typedef ptrdiff_t    difference_type;

	public:
		static T* allocate();
		static T* allocate(size_type _n);

		static void deallocate(T* _ptr);
		static void deallocate(T* _ptr, size_type _n);
	};

	template <class T>
	T* allocator<T>::allocate()
	{
		return static_cast<T*>(::operator new(sizeof(T)));//调用到全局函数
	}

	template <class T>
	T* allocator<T>::allocate(size_type _n)
	{
		if (!_n) { return nullptr; }//经过实践发现，如果new的大小为零，也是会返回一个不为空指针指向大小不为零的内存区域
		return static_cast<T*>(::operator new(_n * sizeof(T)));//调用到全局函数，只有这么调用new 才可以满足这种语法。
	}

	template <class T>
	void allocator<T>::deallocate(T* _ptr)
	{
		if (_ptr)
		{
			::operator delete(_ptr);
		}
	}

	template <class T>
	void allocator<T>::deallocate(T* _ptr, size_type /*size*/)
	{
		deallocate(_ptr);
	}

	////析构之前是获取这段内存的首地址，而传入的往往是指向某一个数据结构的指针，这个时候需要从这个指针里面获取首地址
	template<class T>
	T* get_deleter(T* _ptr) {
		return &*_ptr;
	}
}

