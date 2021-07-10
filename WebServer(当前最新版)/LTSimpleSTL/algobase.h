//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

//该头文件负责实现常见泛型算法。目前已经实现的有：
//max,min,swap
//copy，fill,fill_n,copy_backward,copy_if,copy_n
//equal
//lexicographical_compare。新增一个自定义循环逻辑比较算法lexicographical_loop_compare
//mismatch
#include "iterator.h"
#include "util.h"
#include "functional.h"
#include "type_traits.h"
#include "allocator.h"
#include <iostream>

namespace LT {
	//---------------------------------------------max,min，swap----------------------------------------
	//为了解决循环引用swap定义在util头文件里面
	template <class T>
	const T& min(const T& _a, const T& _b) {
		return (_a < _b) ? _a : _b;
	}

	template <class T, class Compare>
	const T& min(const T& _a, const T& _b, Compare cmp) {
		return (cmp(_a, _b)) ? _a : _b;
	}

	template <class T>
	const T& max(const T& _a, const T& _b) {
		return (_a > _b) ? _a : _b;
	}

	template <class T, class Compare>
	const T& max(const T& _a, const T& _b, Compare cmp) {
		return (cmp(_a, _b)) ? _a : _b;
	}


	

	//----------------------------------三个底层内存管理工具的高阶函数---------------------------------
	//copy,fill
	//-------------------------------------------copy-----------------------------------------------------
	//copy，该函数的输入参数是一组迭代器，可以将输入区间[_first,end)内的元素复制到输出区间[result,result + (end - _first))内
	//保证第二组迭代器所在容器有足够的空间。需要强化效率
	//①萃取迭代器类型
	
	//input_iterator版本
	template<class InputItertor, class OutputIterator>
	inline OutputIterator __copy(InputItertor _first, InputItertor _last, OutputIterator _result, LT::input_iterator_tag) {
		for (; _first != _last; ++_first, ++_result) {
			*_result = *_first;
		}
		return _result;
	}
	//random_iterator版本
	template<class RandomIterator, class OutputIterator, class Distance>
	inline OutputIterator __copy_d(RandomIterator _first, RandomIterator _last, OutputIterator _result, Distance*) {
		//用n来决定循环次数，速度更快。
		for (Distance n = _last - _first; n; --n, ++_result, ++_first) {
			*_result = *_first;
		}
		return _result;
	}
	template<class RandomItertor, class OutputIterator>
	inline OutputIterator __copy(RandomItertor _first, RandomItertor _last, OutputIterator _result, LT::random_access_iterator_tag) {
		for (; _first != _last; ++_first, ++_result) {
			*_result = *_first;
		}
		return _result;
	}
	//原生指针版本
	template<class T>
	inline T* __copy_t(const T* _first, const T* _last, T* _result, true_type) {
		std::memmove(_result, _first, sizeof(T) * (_last - _first));
		return _result + (_last - _first);
	}

	template<class T>
	inline T* __copy_t(const T* _first, const T* _last, T* _result, false_type) {
		return __copy_d(_first, _last, _result, (ptrdiff_t*)0);
	}
	//__copy_dispatch
	template<class InputIterator, class OutputIterator>
	inline OutputIterator __copy_dispatch(InputIterator _first, InputIterator _last, OutputIterator _result) {
		return __copy(_first, _last, _result, LT::iterator_category(_first));
	}

	template<class Tp,class Up>
	inline typename std::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
		std::is_trivially_copy_assignable<Up>::value,
		Up*>::type
	 __copy_dispatch(Tp* _first, Tp* _last, Up* _result) {
		typedef typename _type_traits<Tp>::has_trivial_assignment_operator is_true_type;
		return __copy_t(_first, _last, _result, is_true_type());
	}

	
	//copy对外接口
	template<class InputIterator, class OutputIterator>
	inline OutputIterator copy(InputIterator _itBegin, InputIterator _itEnd, OutputIterator _result) {
		return __copy_dispatch<InputIterator, OutputIterator>(_itBegin, _itEnd, _result);
	}

	//加速重载版本
	inline char* copy(const char* _first, const char* _last, char* _result) {
		std::memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}
	inline wchar_t* copy(const wchar_t* _first, const wchar_t* _last, wchar_t* _result) {
		std::memmove(_result, _first, _last - _first);
		return _result + (_last - _first);
	}


	//-------------------------------------------------------------fill_n--------------------------------------------------------
	//该函数从迭代器开始的位置，填充n个元素
	template<class OutputIterator, class Size_t, class T>
	OutputIterator __fill_n(OutputIterator _first, Size_t _n, const T& _value) {
		for (; _n > 0; --_n, ++_first) {
			*_first = _value;
		}
		return _first;
	}

	// 按字节拷贝的特化版本
	template<class Tp, class Size_t, class Up>
	typename std::enable_if<
		std::is_integral<Tp>::_value && sizeof(Tp) == 1 && //is_integral<T>检查T是否是整数类型
		!std::is_same<Tp, bool>::_value &&
		std::is_integral<Up>::_value && sizeof(Up) == 1,
		Tp*>::type
		__fill_n(Tp* _first, Size_t _n, Up _value)
	{
		if (_n > 0)
		{
			std::memset(_first, (unsigned char)_value, (size_t)(_n));
		}
		return _first + _n;
	}

	template <class OutputIterator, class Size_t, class T>
	OutputIterator fill_n(OutputIterator _first, Size_t _n, const T& _value) {
		return __fill_n(_first, _n, _value);
	}


	//--------------------------------------------------------fill:--------------------------------------------------------------
	//为区间中的元素填充新值
	template <class ForwardIter, class T>
	void __fill(ForwardIter _first, ForwardIter _last, const T& _value, LT::forward_iterator_tag)
	{
		for (; _first != _last; ++_first)
		{
			*_first = _value;
		}
	}

	template <class RandomIter, class T>//支持随机访问说明是连续内存
	void __fill(RandomIter _first, RandomIter _last, const T& _value,LT::random_access_iterator_tag)
	{
		LT::fill_n(_first, _last - _first, _value);
	}

	template <class ForwardIter, class T>
	void fill(ForwardIter _first, ForwardIter _last, const T& _value)
	{
		__fill(_first, _last, _value, iterator_category(_first));
	}

//----------------------------------------------------- copy_backward------------------------------------------------
// 将 [_first, _last)区间内的元素拷贝到 [_endResult - (_last - _first), _endResult)内

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		__copy_backward_dispatch(BidirectionalIter1 _first, BidirectionalIter1 _last,
			BidirectionalIter2 _endResult, LT::bidirectional_iterator_tag)
	{
		while (_first != _last)
		{
			*--_endResult = *--_last;
		}
			
		return _endResult;
	}

	//  random_access_iterator_tag 版本
	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		__copy_backward_dispatch(BidirectionalIter1 _first, BidirectionalIter1 _last,
			BidirectionalIter2 _endResult, LT::random_access_iterator_tag)
	{
		for (auto n = _last - _first; n > 0; --n)
			*--_endResult = *--_last;
		return _endResult;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		__copy_backward(BidirectionalIter1 _first, BidirectionalIter1 _last,
			BidirectionalIter2 _endResult)
	{
		return __copy_backward_dispatch(_first, _last, _endResult,
			iterator_category(_first));
	}

	// 为 trivially_copy_assignable 类型提供特化版本
	template <class Tp, class Up>
	typename LT::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value&&
		std::is_trivially_copy_assignable<Up>::value,
		Up*>::type
		__copy_backward(Tp* _first, Tp* _last, Up* _endResult)
	{
		const auto n = static_cast<size_t>(_last - _first);
		if (n != 0)
		{
			_endResult -= n;
			std::memmove(_endResult, _first, n * sizeof(Up));
		}
		return _endResult;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		copy_backward(BidirectionalIter1 _first, BidirectionalIter1 _last, BidirectionalIter2 _endResult)
	{
		return __copy_backward(_first, _last, _endResult);
	}
	
	//copy_if
	//把[first,end]区间内满足条件的元素拷贝到result为起始的位置
	//unary_pred:是一个一元操作符
	template<class InputIter, class OutputIter, class UnaryPredicate>
	OutputIter copy_if(InputIter _first, InputIter _last, OutputIter _result, UnaryPredicate _unaryPred)
	{
		size_t n = static_cast<size_t>(LT::distance(_first, _last));
		for (; n; --n, ++_first)
		{
			if (_unaryPred(*_first))
			{
				*_result++ = *_first;
			}
		}
	}

	//copy_n
	//把以first为起始的n个元素拷贝到result为起始的位置
	template<class InputIter, class Size, class OutputIter>
	OutputIter __copy_n(InputIter _first, Size _n, OutputIter _result, LT::input_iterator_tag)
	{
		for (; _n; --_n, ++_first, ++_result)
		{
			*_result = *_first;
		}
		return _result;
	}

	template<class InputIter, class Size, class OutputIter>
	OutputIter __copy_n(InputIter _first, Size _n, OutputIter _result, LT::random_access_iterator_tag)
	{
		return LT::copy(_first, _first + _n, _result);
	}

	template<class InputIter, class Size, class OutputIter>
	OutputIter copy_n(InputIter _first, Size _n, OutputIter _result)
	{
		return __copy_n(_first, _n, _result, LT::iterator_category(_first));
	}

	//-----------------------------------------------move-------------------------------------------------------------
	//迭代器版本
	template<class InputIterator, class OutputIterator>
	OutputIterator __move_dispatch(InputIterator _first, InputIterator _last, OutputIterator _result, random_access_iterator_tag)
	{
		size_t n = LT::distance(_first, _last);
		for (; n; --n, ++_first, ++_result)
		{
			*_result = LT::move(*_first);
		}
		return _result;
	}

	template<class InputIterator, class OutputIterator>
	OutputIterator __move_dispatch(InputIterator _first, InputIterator _last, OutputIterator _result, forward_iterator_tag)
	{
		size_t n = LT::distance(_first, _last);
		for (; n; --n, ++_first, ++_result)
		{
			*_result = LT::move(*_first);
		}
		return _result;
	}

	template<class InputIterator, class OutputIterator>
	OutputIterator __move(InputIterator _first, InputIterator _last, OutputIterator _result)
	{
		return __move_dispatch(_first, _last, _result,LT::iterator_category(_first));
	}
	

	//直接按位拷贝的版本
	template<class Tp, class Up>
	typename LT::enable_if <
		std::is_same<typename std::remove_const<Tp>::type, Up>::value && 
		std::is_trivially_move_assignable<Tp>::value,
		Up*>::type
		__move(Tp* _first, Tp* _last, Up* _result)
	{
		const size_t n = static_cast<size_t>(_last - _first);
		n && std::memmove(_result, _first, n * sizeof(Tp));
		return _result + n;
	}

	template<class InputIterator, class OutputIterator>
	OutputIterator move(InputIterator _first, InputIterator _last, OutputIterator _result)
	{
		return __move(_first, _last, _result);
	}
	//-------------------------------------------------------move_backward()-----------------------------------------------
		//迭代器版本
	template<class InputIterator, class OutputIterator>
	OutputIterator __move_backward_dispatch(InputIterator _first, InputIterator _last, OutputIterator _endResult, random_access_iterator_tag)
	{
		size_t n = _last - _first;
		for (; n; --n)
		{
			*--_endResult = LT::move(*--_last);
		}
		return _endResult;
	}

	template<class InputIterator, class OutputIterator>
	OutputIterator __move_backward_dispatch(InputIterator _first, InputIterator _last, OutputIterator _endResult, forward_iterator_tag)
	{
		size_t n = LT::distance(_first, _last);
		for (; n; --n)
		{
			*--_endResult = LT::move(*--_last);
		}
		return _endResult;
	}

	template<class InputIterator, class OutputIterator>
	OutputIterator move_backward(InputIterator _first, InputIterator _last, OutputIterator _endResult)
	{
		return __move_backward_dispatch(_first, _last, _endResult, LT::iterator_category(_first));
	}



	//--------------------------------------------equal--------------------------------------------------
	//该函数负责比较两组迭代器所指的两个区间上的元素值是否相等。
	//两组迭代器可以是不同类型，但是迭代器的value_type要相等。
	template <typename InputIterator1, typename InputIterator2>
	bool equal(InputIterator1 _first1, InputIterator1 _last, InputIterator2 _first2) {
		for (; _first1 != _last; ++_first1, ++_first2) {
			if (*_first1 != *_first2) {
				return false;
			}
		}
		return true;
	}

	template <typename InputIterator1, typename InputIterator2, typename Comp>
	bool equal(InputIterator1 _first1, InputIterator1 _last, InputIterator2 _first2, Comp _cmp) {
		for (; _first1 != _last; ++_first1, ++_first2) {
			if (!_cmp(*_first1  *_first2)) {
				return false;
			}
		}
		return true;
	}

	//-------------------------------------lexicographical_compare-----------------------------------
	//该函数按字典序进行比较，如果遇到第一个满足严格弱序，前面相同的情况下，短序列比长序列小
	template<typename InputIterator1, class InputIterator2>
	bool lexicographical_compare(InputIterator1 _first1, InputIterator1 _end1, InputIterator2 _first2, InputIterator2 _end2) {
		for (; _first1 != _end1 && _first2 != _end2; ++_first1,++_first2) {
			if (*_first1 < *_first2) {
				return true;
			}
			if (*_first2 < *_first1) {
				return false;
			}
		}
		return _first1 == _end1 && _first2 != _end2;
	}

	//自定义比较版本
	template<typename InputIterator1, typename InputIterator2, typename Comp>
	bool lexicographical_compare(InputIterator1 _first1, InputIterator1 _end1, InputIterator2 _first2, InputIterator2 _end2, Comp _cmp) {
		for (; _first1 != _end1 && _first2 != _end2; ++_first1, ++_first2) {
			if (_cmp(*_first1 , *_first2)) {
				return true;
			}
			if (_cmp(*_first2, *_first1)) {
				return false;
			}
		}
		return _first1 == _end1 && _first2 != _end2;
	}

	//加速特化版本,针对字符串
	// 针对 const char* 的特化版本
	/*
	template<>
	bool lexicographical_compare<const char*, const char*>(const  char* _first1, const  char* _last1, const  char* _first2, const char* _last2)
	{
		const auto len1 = _last1 - _first1;
		const auto len2 = _last2 - _first2;
		// 先比较相同长度的部分
		const auto result = memcmp(_first1, _first2, LT::min(len1, len2));
		// 若相等，长度较长的比较大
		return result != 0 ? result < 0 : len1 < len2;
	}

	// 针对 const unsigned char* 的特化版本
	template<>
	bool lexicographical_compare<const unsigned char*, const unsigned char*>(const unsigned char* _first1, const unsigned char* _last1, const unsigned char* _first2, const unsigned char* _last2)
	{
		const auto len1 = _last1 - _first1;
		const auto len2 = _last2 - _first2;
		// 先比较相同长度的部分
		const auto result = memcmp(_first1, _first2, LT::min(len1, len2));
		// 若相等，长度较长的比较大
		return result != 0 ? result < 0 : len1 < len2;
	}
	*/
	/*
	//-------------------------------------lexicographical_loop_compare-----------------------------------
	//该函数按字典序进行比较，如果遇到第一个满足严格弱序，前面相同的情况下，短序列比长序列小
	template<typename InputIterator1, class InputIterator2>
	bool lexicographical_loop_compare(InputIterator1 _first1, InputIterator1 _end1, InputIterator2 _first2, InputIterator2 _end2) {
		for (; _first1 != _end1 && _first2 != _end2; ++_first1, ++_first2) {
			if (*_first1 < *_first2) {
				return true;
			}
			if (*_first2 < *_first1) {
				return false;
			}
		}
		return _first1 == _end1 && _first2 != _end2;
	}

	//自定义比较版本
	template<typename InputIterator1, typename InputIterator2, typename Comp>
	bool lexicographical_loop_compare(InputIterator1 _first1, InputIterator1 _end1, InputIterator2 _first2, InputIterator2 _end2, Comp _cmp) {
		for (; _first1 != _end1 && _first2 != _end2; ++_first1, ++_first2) {
			if (_cmp(*_first1, *_first2)) {
				return true;
			}
			if (_cmp(*_first2, *_first1)) {
				return false;
			}
		}
		return _first1 == _end1 && _first2 != _end2;
	}

	//加速特化版本,针对字符串
	// 针对 const char* 的特化版本

	bool lexicographical_loop_compare(const  char* _first1, const  char* _last1, const  char* _first2, const char* _last2)
	{
		const auto len1 = _last1 - _first1;
		const auto len2 = _last2 - _first2;
		// 先比较相同长度的部分
		const auto result = std::memcmp(_first1, _first2, LT::min(len1, len2));
		// 若相等，长度较长的比较大
		return result != 0 ? result < 0 : len1 < len2;
	}

	// 针对 const unsigned char* 的特化版本
	bool lexicographical_loop_compare(const unsigned char* _first1, const unsigned char* _last1, const unsigned char* _first2, const unsigned char* _last2)
	{
		const auto len1 = _last1 - _first1;
		const auto len2 = _last2 - _first2;
		// 先比较相同长度的部分
		const auto result = std::memcmp(_first1, _first2, LT::min(len1, len2));
		// 若相等，长度较长的比较大
		return result != 0 ? result < 0 : len1 < len2;
	}
	*/


	//-----------------------------------------------mismatch------------------------------------------------------------
	//比较两个序列，找到两个序列中第一个不同的地方
	//返回两个迭代器分别指向两个序列中不同的元素
	template<class InputIter1, class InputIter2>
	LT::pair<InputIter1, InputIter2> mismatch(InputIter1 _first1, InputIter1 _end1, InputIter2 _first2)
	{
		size_t n = static_cast<size_t> (LT::distance(_first1, _end1));
		for (; n; --n)
		{
			if (*_first1 != *_first2) { break; }
			else { 
				++_first1;
				++_first2;
			}
		}

		return LT::make_pair(_first1, _first2);
	}

	template<class InputIter1, class InputIter2, class Comp>
	LT::pair<InputIter1, InputIter2> mismatch(InputIter1 _first1, InputIter1 _end1, InputIter2 _first2, Comp _cmp)
	{
		size_t n = static_cast<size_t> (LT::distance(_first1, _end1));
		for (; n; --n)
		{
			if (!_cmp(*_first1, *_first2)) { break; }
			else {
				++_first1;
				++_first2;
			}
		}

		return LT::make_pair(_first1, _first2);
	}
}

