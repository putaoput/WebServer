//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ��ǿռ�������֮һ���򵥵Ķ�new ��delete�����˷�װ������ΪĬ�Ͽռ�������
//ֻ�����ڴ�
#include "construct.h"

namespace LT {
	//
	template <class T>
	class allocator
	{
	public://������ȡ����
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
		return static_cast<T*>(::operator new(sizeof(T)));//���õ�ȫ�ֺ���
	}

	template <class T>
	T* allocator<T>::allocate(size_type _n)
	{
		if (!_n) { return nullptr; }//����ʵ�����֣����new�Ĵ�СΪ�㣬Ҳ�ǻ᷵��һ����Ϊ��ָ��ָ���С��Ϊ����ڴ�����
		return static_cast<T*>(::operator new(_n * sizeof(T)));//���õ�ȫ�ֺ�����ֻ����ô����new �ſ������������﷨��
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

	////����֮ǰ�ǻ�ȡ����ڴ���׵�ַ���������������ָ��ĳһ�����ݽṹ��ָ�룬���ʱ����Ҫ�����ָ�������ȡ�׵�ַ
	template<class T>
	T* get_deleter(T* _ptr) {
		return &*_ptr;
	}
}

