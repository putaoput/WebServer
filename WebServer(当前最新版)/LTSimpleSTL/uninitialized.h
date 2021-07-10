//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ�����ʵ��uninitialized_copy(),uninitialized_fill_n(),uninitialized_fiil()�����ڴ������
//ʵ����֧����ֵ���õ�uninitialized_move()
//���ڴ��ڶԷǻ����������͵ļ��ٴ�������һ��Ҫ��֤�����һ��Output Interator��������Ӧ��һ�������ڴ�
//���������Ͳ�εĺ�����Ӧ�����߲�κ���:copy(),fill(),fill_n()��
//ʵ��������Ҫ�쳣�����
//Ҫ����лع����ܣ���commit or rollback��Ҫôȫ�����죬Ҫôһ��Ҳ������
//����POD���Ͳ������Ч�÷�ʽ�����ڷ�POD���Ͳ�����յķ�ʽ
#include "construct.h"
#include "memory.h"
#include "iterator.h"
#include "type_traits.h"
#include "algobase.h" //�����и��߽׵��ڴ���亯����������������������͵��ڴ�
#include "util.h"
#include <iostream>
#


namespace LT {
	
	//-------------------------------------------uninitialized_fiil_n()��ʵ��---------------------------------------------
	
	//true_type����������ģ�庯��
	template <typename ForwardIterator, class Size, class T>
	inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator _first, Size _n, const T& _value, true_type) {
		return LT::fill_n(_first, _n, _value);//�������߽׵ĺ���ִ��
	}
	//__flase_type����������ģ�庯��
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
			//�˴�Ӧ�����쳣����,�Ա�֤commit or rollback��
			for (auto it = _first; cur != it; ++ it) {
				destroy(&*it);
			}
			return _first;
		}	
	}

	template <typename ForwardIterator, class Size, class T, class Val_t>
	inline ForwardIterator __uninitialized_fill_n(ForwardIterator _first, Size _n, const T& _value, Val_t*) {
		typedef typename _type_traits<T>::is_POD_type is_POD;//��һ������ȡ���Ƿ��ǻ����������ͣ������struct true_type ����false_type
		return __uninitialized_fill_n_aux(_first, _n, _value, is_POD());
		//��������true_type ���� false_type ���͵���ͬ��ģ�庯����
	}


	//�ú����Ľ����߼���������ȡ����������value_type��Ȼ���жϸ�value_type�Ƿ���POD�ͱ�
	template <typename ForwardIterator, class Size, class T>
	inline ForwardIterator uninitialized_fill_n(ForwardIterator _first, Size _n, const T& _value) {
		return __uninitialized_fill_n(_first, _n, _value, value_type(_first));
		//������ȡ���ͣ�value_type��ȡ��������ʵ�Ǹ����͵�ָ�롣
	}


	//-------------------------------------------uninitialized_copy()��ʵ��---------------------------------------------
	template <typename InputIterator, class ForwardIterator>//������������
	inline ForwardIterator __uninitialized_copy_aux(InputIterator _first, InputIterator _last, ForwardIterator _result, true_type) {
		return LT::copy(_first, _last, _result);//���ɸ߽׺�������
	}

	template <typename InputIterator, class ForwardIterator>//�ǻ�����������
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
			//�˴�Ӧ�����쳣����,�Ա�֤commit or rollback��
			for (auto it = _result; cur != it; ++it) {
				destroy(&*it);
			}
			return _result;
		}
	}

	//Ҫ��char* �� wchar_t*�ṩ��Ϊ��Ч���ػ��汾

	inline char* uninitialized_copy(const char* _first, const char* _last, char* _result) {
		std::memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}

	inline wchar_t* uninitialized_copy(const wchar_t* _first, const wchar_t* _last, wchar_t* _result) {
		memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}

	template <typename InputIterator, class ForwardIterator, class T>//�ֱ��Ƿ��ǻ�����������
	inline ForwardIterator __uninitialized_copy(InputIterator _first, InputIterator _last, ForwardIterator _result, T*) {
		typedef typename _type_traits<T>::is_POD_type is_POD;
		return __uninitialized_copy_aux(_first, _last, _result, is_POD());
	}

	

	//�������ṩһ�鿽�������������һ����������ݿ�������һ������,���ؿ���������λ�õĵ�����
	template <typename InputIterator, class ForwardIterator>//������������
	inline ForwardIterator uninitialized_copy(InputIterator _first, InputIterator _last, ForwardIterator _result) 
	{
		return __uninitialized_copy(_first, _last, _result, value_type(_result));//��ȡvalue_type
	}


	//-------------------------------------------uninitialized_fill()��ʵ��---------------------------------------------

	template <typename ForwardIterator, class T>
	inline void __uninitialized_fill_aux(ForwardIterator _first, ForwardIterator _last, const T& _value, true_type) {
		fill(_first, _last, _value);//����stl�㷨fill().
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
			//ʧ����Ҫ��������
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


