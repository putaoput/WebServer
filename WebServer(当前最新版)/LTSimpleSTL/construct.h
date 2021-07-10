//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//���ͷ�ļ����ڹ�������������������ڸ������ڴ������Ϲ�����󣬺͸������ڴ���������������
//������Ҫ���ǣ���������ջ��������ڴ档�ṩ������ڴ�������е�������
//�ǵڶ�����д��ͷ�ļ���

#include <new>
#include "type_traits.h"
#include "util.h"


namespace LT {
	//------------------------------------------------------construct-----------------------------------
	template<class T1>
	inline void construct(T1* _ptr1) {
		new(_ptr1) T1();//��_ptr1��ָ���ڴ�ռ䴴��һ������ΪT1�Ķ���
	}

	template<class T1, class T2>
	inline void construct(T1* _ptr1, const T2& _value) {
		new(_ptr1) T1(_value);
	}

	//��ֵ���õĹ��캯��
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