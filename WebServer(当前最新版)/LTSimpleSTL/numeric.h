//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

#include "iterator.h"
//��ͷ�ļ�����ʵ��STL�����ֵ�㷨��Ŀǰ�Ѿ�ʵ�ֵ��У�
//

namespace LT {

	/***********************************************************************************/
	// accumulate
	// ��ÿ��Ԫ�ؽ������ / �����Ƕ�ÿ��Ԫ�ؽ��ж�Ԫ����
	/**********************************************************************************/
	template <class InputIter, class T>
	T accumulate(InputIter _first, InputIter _last, T _initValue)
	{
		for (; _first != _last; ++_first)
		{
			_initValue += *_first;
		}
		return _initValue;
	}

	template <class InputIter, class T, class BianaryOp>
	T accumulate(InputIter _first, InputIter _last, T _initValue, BianaryOp _bianaryOp)
	{
		for (; _first != _last; ++_first)
		{
			_initValue = _bianaryOp(_initValue, *_first);
		}
		return _initValue;
	}

	/***********************************************************************************/
	// adjacent_difference
	// ��������Ԫ�صĲ�ֵ/�����Ƕ�����Ԫ�ؽ��ж�Ԫ������������浽��_resultΪ��ʼ������
	/**********************************************************************************/
	template <class InputIter, class OutputIter>
	OutputIter adjacent_difference(InputIter _first, InputIter _last, OutputIter _result)
	{
		if (_first == _last)
		{
			return _result;
		}

		*_result = *_first;  // ��¼��һ��Ԫ��
		typename iterator_traits<InputIter>::value_type value = *_first;
		while (++_first != _last)
		{
			typename iterator_traits<InputIter>::value_type tmp = *_first;
			*++_result = tmp - value;
			value = tmp;
		}
		return ++_result;
	}

	template <class InputIter, class OutputIter, class BinaryOp>
	OutputIter adjacent_difference(InputIter _first, InputIter _last, OutputIter _result, BinaryOp _binaryOp)
	{
		if (_first == _last)
		{
			return _result;
		}

		*_result = *_first;  // ��¼��һ��Ԫ��
		typename iterator_traits<InputIter>::value_type value = *_first;
		while (++_first != _last)
		{
			typename iterator_traits<InputIter>::value_type tmp = *_first;
			*++_result = _binaryOp(tmp, value);
			value = tmp;
		}
		return ++_result;
	}

	/*****************************************************************************************/
	// inner_product
	// �� _initValue Ϊ��ֵ����������������ڻ�//�Զ������
	/*****************************************************************************************/
	// �汾1
	template <class InputIter1, class InputIter2, class T>
	T inner_product(InputIter1 _first1, InputIter1 _last1, InputIter2 _first2, T _initValue)
	{
		for (; _first1 != _last1; ++_first1, ++_first2)
		{
			_initValue = _initValue + (*_first1 * *_first2);
		}
		return _initValue;
	}

	// �汾2
	template <class InputIter1, class InputIter2, class T, class BinaryOp1, class BinaryOp2>
	T inner_product(InputIter1 _first1, InputIter1 _last2, InputIter2 _first2, T _initValue,
		BinaryOp1 binary_op1, BinaryOp2 _binaryOp2)
	{
		for (; _first1 != _last2; ++_first1, ++_first2)
		{
			_initValue = binary_op1(_initValue, _binaryOp2(*_first1, *_first2));
		}
		return _initValue;
	}

	/*****************************************************************************************/
	// iota
	// ���[_first, _last)���� initValue Ϊ��ֵ��ʼ����
	/*****************************************************************************************/
	template <class ForwardIter, class T>
	void iota(ForwardIter _first, ForwardIter _last, T _initValue)
	{
		while (_first != _last)
		{
			*_first++ = _initValue;
			++_initValue;
		}
	}

	/*****************************************************************************************/
	// partial_sum
	// ����ֲ��ۼ���� / �Զ����Ԫ������������浽�� _result Ϊ��ʼ��������
	/*****************************************************************************************/
	template <class InputIter, class OutputIter>
	OutputIter partial_sum(InputIter _first, InputIter _last, OutputIter _result)
	{
		if (_first == _last)  return _result;
		*_result = *_first;  // ��¼��һ��Ԫ��
		auto value = *_first;
		while (++_first != _last)
		{
			value = value + *_first;
			*++_result = value;
		}
		return ++_result;
	}

	template <class InputIter, class OutputIter, class BinaryOp>
	OutputIter partial_sum(InputIter _first, InputIter _last, OutputIter _result,
		BinaryOp binary_op)
	{
		if (_first == _last)  return _result;
		*_result = *_first;  //��¼��һ��Ԫ��
		auto value = *_first;
		while (++_first != _last)
		{
			value = binary_op(value, *_first);
			*++_result = value;
		}
		return ++_result;
	}
}