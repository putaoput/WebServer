//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件提供迭代器
#include <cstddef>
#include "type_traits.h"

namespace LT {
	//迭代器分为只读，只写，前向，双向，随机访问五种类型。
	
	struct input_iterator_tag {};
	struct output_iterator_tag{};
	struct forward_iterator_tag: public input_iterator_tag,output_iterator_tag{};
	struct bidirectional_iterator_tag: public forward_iterator_tag{};
	struct random_access_iterator_tag: public bidirectional_iterator_tag{};


	template <class C, class V, class D = ptrdiff_t, class P = V*, class R = V&>
	struct iterator {
		typedef C                            iterator_category;
		typedef V                            value_type;
		typedef P                            pointer;
		typedef R                            reference;
		typedef D                            difference_type;
	};


	//--------------------------------------------------判断是否是迭代器-------------------------------------------------------
	//如果不是迭代器，那么返回值是short类型，如果是迭代器，那么返回值是char类型，一个是2字节，一个是1字节


	template <class T>
	struct is_iter_cat
	{
	private:
		template <class U> static short test(...);
		template <class U> static char test(typename U::iterator_category* = 0);
	public:
		static const bool value = sizeof(test<T>(0)) == sizeof(char);
	};


	
	template <class Iterator, bool>
	struct iterator_traits_impl {};

	template <class Iterator>
	struct iterator_traits_impl<Iterator, true>
	{
		typedef typename Iterator::iterator_category iterator_category;
		typedef typename Iterator::value_type        value_type;
		typedef typename Iterator::pointer           pointer;
		typedef typename Iterator::reference         reference;
		typedef typename Iterator::difference_type   difference_type;
	};

	template <class Iterator, bool>
	struct iterator_traits_helper {};

	template <class Iterator>
	struct iterator_traits_helper<Iterator, true>
		: public iterator_traits_impl<Iterator,
		std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
		std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value>
	{
	};


	// 萃取迭代器的特性
	template <class Iterator>
	struct iterator_traits
		: public iterator_traits_helper<Iterator, is_iter_cat<Iterator>::value> {};

	//
	//针对原生指针的偏特化版本
	template <class T>
	struct iterator_traits<T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T                                    value_type;
		typedef T*									 pointer;
		typedef T&					   				 reference;
		typedef ptrdiff_t                            difference_type;
	};

	//针对原生cosnt 指针的偏特化版本
	template <class T>
	struct iterator_traits<const T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T                                    value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef ptrdiff_t                            difference_type;
	};

	//用于确定迭代器的类型
	template <class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category //此处typename用于标明内嵌依赖类型名
		iterator_category(const Iterator&) {
			typedef typename iterator_traits<Iterator>::iterator_category category;
			return category();
	}

	// 用于确定迭代器的 distance_type
	template <class Iterator>
	typename iterator_traits<Iterator>::difference_type*
		distance_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

	// 用于确定迭代器的 value_type
	template <class Iterator>
	typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	//-----------------------------------------distance函数----------------------------------
	template <class InputIterator>
	typename iterator_traits<InputIterator>::difference_type
		__distance(InputIterator _first, InputIterator _last, input_iterator_tag)
	{
		typename iterator_traits<InputIterator>::difference_type n = 0;
		while (_first != _last)
		{
			++_first;
			++n;
		}
		return n;
	}

	// distance 的 random_access_iterator 的版本
	template <class RandomIter>
	typename iterator_traits<RandomIter>::difference_type
		__distance(RandomIter _first, RandomIter _last,
			random_access_iterator_tag)
	{
		return _last - _first;
	}

	// distance函数
	template <class InputIterator>
	typename iterator_traits<InputIterator>::difference_type
		distance(InputIterator _first, InputIterator _last)
	{
		return __distance(_first, _last, iterator_category(_first));
	}


	//---------------------------------------移动函数-------------------------------------------------
	//迭代器的移动函数，前进n个距离
	template <class InputIterator, class Distance>
	inline void __advance(InputIterator& _it, Distance _n, input_iterator_tag) {
		while (--_n) {
			++_it;
		}
	}

	template <class BidirectionalIterator, class Distance>
	inline void __advance(BidirectionalIterator& _it, Distance _n, bidirectional_iterator_tag) {
		if (_n >= 0) {
			while (--_n) {
				++_it;
			}
		}
		else {
			while (++_n) {
				--_it;
			}
		}
	}

	template <class RandomAcessIterator, class Distance>
	inline void __advance(RandomAcessIterator& _it, Distance _n, random_access_iterator_tag) {
		_it += _n;
	}

	template <class InputIterator, class Distance>
	inline void advance(InputIterator& _it, Distance _n) {
		__advance(_it, _n, iterator_category(_it));
	}

	//----------------------------------------反向迭代器------------------------------------------------
	template <class Iterator>
	class reverse_iterator 
	{
	private:
		Iterator cur;
	public:
		// 反向迭代器的五种相应型别
		typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
		typedef typename iterator_traits<Iterator>::value_type        value_type;
		typedef typename iterator_traits<Iterator>::difference_type   difference_type;
		typedef typename iterator_traits<Iterator>::pointer           pointer;
		typedef typename iterator_traits<Iterator>::reference         reference;

		typedef Iterator											  F_Iterator;
		typedef reverse_iterator<Iterator>							  R_Iterator;

	public:
		reverse_iterator(){}
		explicit reverse_iterator(F_Iterator _it) :cur(_it) {};
		//拷贝构造函数
		reverse_iterator(const R_Iterator& __rhs) :cur(__rhs.cur) {};

	public:
		//取出正向迭代器
		F_Iterator get_cur()const {
			return cur;
		}

		//重载操作符 //!!!!注意!!!!因为反向迭代器的rbegin是由正向迭代器的end()初始化而来，所以每次取值要后退一个元素才能取值
		reference operator*() const {
			Iterator tmp = cur;
			return *(--tmp);
		}

		pointer operator->()const {
			return cur;
		}

		//移动操作符----------------------------------------------------------------------------
		R_Iterator& operator++() {
			--cur;
			return *this;
		}

		R_Iterator& operator--() {
			++cur;
			return *this;
		}

		R_Iterator& operator++(int) {
			R_Iterator tmp = *this;
			--cur;
			return tmp;
		}

		R_Iterator& operator--(int) {
			R_Iterator tmp = *this;
			++cur;
			return tmp;
		}

		//+,+=, -, -=, []
		R_Iterator& operator-=(difference_type _n) {
			cur += _n;
			return *this;
		}

		R_Iterator operator-(difference_type _n) {
			return R_Iterator(cur + _n);
		}

		R_Iterator& operator+=(difference_type _n) {
			cur -= _n;
			return *this;
		}

		R_Iterator operator+(difference_type _n) {
			return R_Iterator(cur - _n);
		}

		reference operator[](difference_type _n)const
		{
			return *(*this + _n);
		}
	};


	//----------------------------------------需要在外部重载的操作符-------------------------------------------------

	// 重载 operator-
	template <class Iterator>
	typename reverse_iterator<Iterator>::difference_type
		operator-(const reverse_iterator<Iterator>& _lhs,
			const reverse_iterator<Iterator>& _rhs)
	{
		return _rhs.get_cur() - _lhs.get_cur();
	}

	// 重载比较操作符
	template <class Iterator>
	bool operator==(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return _lhs.get_cur() == _rhs.get_cur();
	}

	template <class Iterator>
	bool operator<(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return _rhs.get_cur() < _lhs.get_cur();
	}

	template <class Iterator>
	bool operator!=(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return !(_lhs == _rhs);
	}

	template <class Iterator>
	bool operator>(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return _rhs < _lhs;
	}

	template <class Iterator>
	bool operator<=(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return !(_rhs < _lhs);
	}

	template <class Iterator>
	bool operator>=(const reverse_iterator<Iterator>& _lhs,
		const reverse_iterator<Iterator>& _rhs)
	{
		return !(_lhs < _rhs);
	}

	//---------------------------------------------------迭代器类型判别---------------------------------------------------
	
	
	
	template <class T, class U, bool = is_iter_cat<iterator_traits<T>>::value>
	struct is_iter_cat_of
		:public LT::bool_constant<
		std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value //用来指示是否可以进行类型转换
		>
	{

	};
	template <class T, class U>
	struct is_iter_cat_of<T, U, false> : public false_type {};
	
	template <class Iter>
	struct is_input_iterator : public is_iter_cat_of<Iter, input_iterator_tag> {};

	template <class Iter>
	struct is_output_iterator : public is_iter_cat_of<Iter, output_iterator_tag> {};

	template <class Iter>
	struct is_forward_iterator : public is_iter_cat_of<Iter, forward_iterator_tag> {};

	template <class Iter>
	struct is_bidirectional_iterator : public is_iter_cat_of<Iter, bidirectional_iterator_tag> {};

	template <class Iter>
	struct is_random_access_iterator : public is_iter_cat_of<Iter, random_access_iterator_tag> {};

	template <class Iter>
	struct is_iterator :
		public bool_constant<is_input_iterator<Iter>::value ||
		is_output_iterator<Iter>::value>
	{
	};

}
