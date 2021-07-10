//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件提供序列式容器list。
#include "iterator.h"
#include "allocator.h"
#include "memory.h"
#include "algobase.h"
#include "assert.h"
#include "memory.h"

namespace LT {
	///**************************************************************************************************************
	//----------------------------------------------list的结点-------------------------------------------------------
	//先设计一个list的结点
	template<class T>
	struct __list_node
	{
		typedef __list_node<T>*  list_node_pointer;
		list_node_pointer pre;
		list_node_pointer next;
		T			 data;
		__list_node() = default;//写这些构造函数加速构造，方便右值构造
		__list_node(const T& _value) :data(_value){}
		__list_node(T&& _value):data(LT::move(_value)){}
		__list_node(const __list_node& _rhs)
			:pre(_rhs.pre), next(_rhs.next), data(_rhs.data){}
		__list_node& operator=(const __list_node& _rhs)
		{
			pre = _rhs.pre;
			next = _rhs.data;
			data = _rhs.data;
		}
		__list_node& operator=(__list_node&& _rhs)
		{
			pre = _rhs.pre;
			next = _rhs.data;
			data = LT::move(_rhs.data);
			_rhs.pre = nullptr;
			_rhs.next = nullptr;
		}
	};

	//**************************************************************************************************************
	//----------------------------------------------list的迭代器-------------------------------------------------------

	template<class T, class Ref, class Ptr>
	struct __list_iterator :public iterator<bidirectional_iterator_tag, T>
	{
		
		typedef typename iterator<bidirectional_iterator_tag,T>::iterator_category                             iterator_category;
		typedef T																							   value_type;
		typedef value_type*																					   pointer;
		typedef value_type&																				       reference;
		typedef typename iterator<bidirectional_iterator_tag, T>::difference_type			                           difference_type;
		
		typedef __list_node<T>*						 node_pointer;
		typedef __list_node<T>						 node;
		typedef size_t                               size_type;
		typedef __list_iterator<T, Ref, Ptr>	     self;
		typedef __list_iterator<T, T&, T*>			 iterator;
		

		//指向一个list结点的指针
		node_pointer nodePtr_;


		//---------------------------------------构造系列函数------------------------------------------------------------------------
		__list_iterator() = default;	//给用户定义的构造函数，即使它什么也不做，使得类型不是聚合，也不是微不足道的。如果您希望您的类是聚合类型或普通类型（或通过传递性，POD类型），那么需要使用’= default’。
										//使用’ = default’也可以与复制构造函数和析构函数一起使用。例如，空拷贝构造函数与默认拷贝构造函数（将执行其成员的复制副本）不同。对每个特殊成员函数统一使用’ = default’语法使代码更容易阅读。
		__list_iterator(const node_pointer& _nodePtr):nodePtr_(_nodePtr){}
		__list_iterator(const iterator& _rhs):nodePtr_(_rhs.nodePtr_){}
		__list_iterator(iterator&& _rhs) :nodePtr_(_rhs.nodePtr_) { _rhs.nodePtr_ = nullptr; }
		__list_iterator(const node& _node):nodePtr_(&_node){}
		self& operator=(const iterator& _rhs) {
			nodePtr_ = _rhs.nodePtr_;
			return *this;
		}
		self& operator=(iterator&& _rhs) {
			nodePtr_ = _rhs.nodePtr_;
			_rhs.nodePtr_ = nullptr;
			return *this;
		}
		~__list_iterator() = default;

		//--------------------------------------对外接口--------------------------------------------------------------------------------
		reference operator*() const
		{
			return nodePtr_->data; 
		}
		pointer operator->() const 
		{ 
			return &(operator*()); 
		}

		self& operator++()
		{
			nodePtr_ = nodePtr_->next;
			return *this;
		}
		self operator++(int)
		{
			self tmp = *this;
			++* this;
			return tmp;
		}
		self& operator--()
		{
			nodePtr_ = nodePtr_->pre;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			--* this;
			return tmp;
		}

		bool operator==(const self& rhs) const 
		{ 
			return nodePtr_ == rhs.nodePtr_;
		}

		bool operator!=(const self& rhs) const
		{ 
			return nodePtr_ != rhs.nodePtr_;
		}

	};
	//*****************************************************************************************************************
	//---------------------------------------------------list-------------------------------------------------------
	template <class T, class Alloc = allocator<T>>
	class list {
	public:
		//照例先定义一波
		typedef T													value_type;
		typedef value_type*											pointer;
		typedef value_type&											reference;
		typedef size_t												size_type;
		typedef __list_iterator<T, T*, T&>							iterator;
		typedef __list_iterator<T, const T*, const T&>				const_iterator;
		typedef __list_node<T>										list_node;
		typedef list_node*											list_node_pointer;
		typedef LT::reverse_iterator<iterator>						reverse_iterator;
		typedef LT::reverse_iterator<const_iterator>				const_reverse_iterator;

		//定义数据
	protected:
		list_node_pointer nodePtr_;//这个指针指向list末尾的一个空结点。通过这个指针来表示整个环状链表
		
		//***************************************************************************************************************************
		//***************************************************对外接口****************************************************************
		//***************************************************************************************************************************
	public:
		//构造函数
		list() { __init(); };
		explicit list(size_type _n) { __init_n(_n, T()); }
		list(size_type _n, const value_type& _value) { __init_n(_n, _value); }
		template<class InputIter,
				typename std::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
		list(InputIter _itBeign, InputIter _itEnd) { __init_iter(_itBeign, _itEnd); }
		list(std::initializer_list<T> _ilist) { __init_iter(_ilist.begin(), _ilist.end()); }
		list(const list& _rhs)
		{
			//__init_iter(_rhs.begin(), _rhs.begin());
		}
		list(list&& _rhs) :nodePtr_(_rhs.nodePtr_) { _rhs.nodePtr_ = nullptr; }
		list& operator=(const list& _rhs) {
			if (*this != _rhs)
			{
				//__assign_iter(begin(), end());
			}
			return *this;
		}
		list& operator=(list&& _rhs) 
		{ 
			swap(_rhs);
			return *this; 
		}
		list& operator=(std::initializer_list<T> _ilist)
		{
			list tmp(_ilist.begin(), _ilist.end()); 
			swap(tmp); 
			return *this;
		}
		~list() 
		{	
			//移动语义的存在使得nodePtr可能为空指针
			if (nodePtr_)
			{
				__uninit();
			}
			
		}

		//------------------------------------------------------迭代器----------------------------------------------------------------
		iterator begin() { return static_cast<iterator>(nodePtr_->next); }//因为迭代器内部只有一个list_node_pointer,所以可以直接进行静态类型转换
		const_iterator cbegin() { return static_cast<const_iterator>(nodePtr_->next); }
		reverse_iterator rbegin() { return reverse_iterator(end()); }//因为迭代器内部只有一个list_node_pointer,所以可以直接进行静态类型转换
		const_reverse_iterator crbegin() { return reverse_iterator(cend()); }

		iterator end() { return static_cast<iterator>(nodePtr_); }//因为迭代器内部只有一个list_node_pointer,所以可以直接进行静态类型转换
		const_iterator cend() { return static_cast<const_iterator>(nodePtr_); }
		reverse_iterator rend() { return reverse_iterator(begin()); }//因为迭代器内部只有一个list_node_pointer,所以可以直接进行静态类型转换
		const_reverse_iterator crend() { return reverse_iterator(cbegin()); }
		
		reference front() { assert(!empty()); return *begin(); }
		reference back() { assert(!empty()); return (nodePtr_->pre->data); }


		//-----------------------------------------------------容量相关-------------------------------------------------------------
		size_type size() { return static_cast<size_type>(LT::distance(begin(), end())); }
		bool empty() const{ return nodePtr_->next == nodePtr_; }
		size_type max_size() const{return static_cast<size_type>(-1);}

		void resize(size_type _newSize){ __resize(_newSize, T()); }
		void resize(size_type _newSize, const value_type& _value) { __resize(_newSize, _value); }
		//----------------------------------------------------节点操作---------------------------------------------------------------
		void push_back(const value_type& _value) { insert(end(), _value); }
		void push_back(const value_type&& _value) { __emplace(end(), _value); }

		void push_front(const value_type& _value) { insert(begin(), _value); }
		void push_front(const value_type&& _value) { __emplace(begin(), _value); }

		void pop_front()
		{
			assert(!empty());
			list_node_pointer nodeToDelete = nodePtr_->next;
			nodePtr_->next = nodeToDelete->next;
			nodeToDelete->next->pre = nodePtr_;
			__destroy_one_node(nodeToDelete);
			__deallocate_one_node_mem(nodeToDelete);
		}
		void pop_back()
		{
			assert(!empty());
			list_node_pointer nodeToDelete = nodePtr_->pre;
			nodePtr_->pre = nodeToDelete->pre;
			nodeToDelete->pre->next = nodePtr_;
			__destroy_one_node(nodeToDelete);
			__deallocate_one_node_mem(nodeToDelete);
		}

		template<class ...Args>
		void emplace(iterator _pos, Args&&... _value) { __emplace(_pos,LT::forward<Args>(_value)...); }
		template <class ...Args>
		void emplace_front(Args&& ..._args) { __emplace(end(), LT::forward<Args>(_args)...); }
		template<class ...Args>
		void emplace_back(Args&& ..._args) { __emplace(begin(), LT::forward<Args>(_args)...); }

		void assign(size_type _n,const value_type& _value) { __assignN(_n, _value); }
		template<class InputIter,
			typename std::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
		void assign(InputIter _itBegin, InputIter _itEnd) { __assign_iter(_itBegin, _itEnd); }
		void assign(std::initializer_list<T> _ilist) { __assign_iter( _ilist.begin(), _ilist.end()); }

		iterator insert(iterator _pos, const value_type& _value)
		{
			//insert返回插入元素的地址
			auto preNode = _pos.nodePtr_->pre;
			auto newNode = __allocate_one_node_mem();
			__construct_one_node(newNode, _value, preNode, _pos.nodePtr_);
			preNode->next = newNode;
			_pos->pre = newNode;
			return static_cast<iterator> (newNode);
		}
		iterator insert(iterator _pos, size_type _n, const value_type& _value)
		{
			auto preNode = _pos.nodePtr_->pre;
			
			list_node_pointer newNode (_pos.nodePtr_->pre);
			while (_n) {
				newNode = __allocate_one_node_mem();
				__construct_one_node(newNode, _value, preNode, _pos.nodePtr_);
				preNode->next = newNode;
				newNode->pre = preNode;
				preNode = newNode;	
				--_n;
			}
			newNode->next = _pos.nodePtr_;
			_pos.nodePtr_ ->pre = newNode;
			return newNode->next;
		}
		iterator insert(iterator _pos, value_type&& _value) 
		{
			list_node_pointer preNode = _pos.nodePtr_->pre;
			list_node_pointer newNode = __allocate_one_node_mem();//使用右值引用版本
			__construct_one_node(newNode, LT::move(_value), preNode, _pos.nodePtr_);
			preNode->next = newNode;
			_pos.nodePtr_->pre = newNode;
			return static_cast<iterator>(newNode);
		}
		template<class InputIterator,
				typename std::enable_if<LT::is_input_iterator<InputIterator>::value, int>::type = 0>
		iterator insert(iterator _pos, InputIterator _itBegin, InputIterator _itEnd) 
		{
			auto preNode = _pos.nodePtr_->pre;
			list_node_pointer newNode(_pos.nodePtr_->pre);
			size_type n = LT::distance(_itBegin, _itEnd);
			while (n) {
				newNode = __allocate_one_node_mem();
				__construct_one_node(newNode, *_itBegin, preNode, _pos.nodePtr_);
				++_itBegin;
				preNode->next = newNode;
				newNode->pre = preNode;
				preNode = newNode;
				--n;
			}
			newNode->next = _pos.nodePtr_;
			_pos.nodePtr_->pre = newNode;
			return newNode->next;
		}

		iterator erase(iterator _pos) 
		{
			assert(_pos != end());
			list_node_pointer preNode = _pos.nodePtr_->pre;
			list_node_pointer nextNode = _pos.nodePtr_->next;
			preNode->next = preNode;
			nextNode->pre = nextNode;
			__destroy_one_node(_pos.nodePtr_);
			__deallocate_one_node_mem(_pos.nodePtr_);
			return static_cast<iterator>(nextNode);
		}
		iterator erase(iterator _itBeg, iterator _itEnd)
		{
			list_node_pointer preNode = _itBeg.nodePtr_->pre;
			list_node_pointer nextNode = _itEnd.nodePtr_;
			preNode->next = preNode;
			nextNode->pre = nextNode;
			while (_itBeg != _itEnd) {
				iterator tmp = _itBeg++;
				__destroy_one_node(tmp.nodePtr_);
				__deallocate_one_node_mem(tmp.nodePtr_);
			}
			
			return static_cast<iterator>(nextNode);
		}

		void clear() {//清除并析构除了尾结点外的其他所有节点
			list_node_pointer cur = nodePtr_->next;
			while (cur != nodePtr_) {
				list_node_pointer tmp = cur;
				cur = cur->next;
				__destroy_one_node(tmp);
				__deallocate_one_node_mem(tmp);
			}
			nodePtr_->next = nodePtr_;
			nodePtr_->pre = nodePtr_;
		}
		void remove(const value_type& _value) {//移除所有值为_value的链表节点。
			iterator cur = begin();
			while (cur != end()) {
				if (*cur == _value) {
					iterator tmp = cur++;
					erase(tmp);
				}
				else {
					++cur;
				}
			}
		}
		template<class Op>
		void remove_if(Op _op)
		{
			iterator it = begin();
			while (it != end())
			{
				if (_op(*it)) {
					auto tmp = it;
					++it;
					erase(tmp);
				}
			}
		}

		//去除所有连续的重复元素，比如n个连续1，最后只剩一个连续1
		void unique() 
		{
			iterator next = begin();
			iterator cur = next++;
			while (next != end()) {
				if (*cur == *next) 
				{
					erase(next);
				}
				else {
					cur = next;
				}
				next = cur;
				++next;
			}
		}
		template<class BianaryPredicate>
		void unique(BianaryPredicate _pred)
		{
			iterator next = begin();
			iterator cur = next++;
			while (next != end()) {
				if (_pre(*cur, *next))
				{
					erase(next);
				}
				else {
					cur = next;
				}
				next = cur + 1;
			}

		}
		void swap(list& _rhs)
		{
			LT::swap(nodePtr_, _rhs.nodePtr_);
		}
		//-------------------------------------------独有接口-----------------------------------------------------------------------
		//splice
		//将一个链表接到该链表的_pos所指位置之前。_otherList必须不同于 *this。执行该操作后，_otherList将变成空链表。
		void splice(iterator _pos, list& _otherList) 
		{
			if (!_otherList.empty()) {
				__transfer_(_pos, _otherList.begin(), _otherList.end());
			}
		}
		//将it所指的元素，接到_pos前面
		void splice(iterator _pos, list&, iterator _it){
			iterator endIt = _it;
			++endIt;
			if (_pos != _it && _pos != endIt) {
				__transfer_(_pos, _it, endIt);
			}
		}
		//将迭代器所指区域接到_pos前面
		void splice(iterator _pos, list&, iterator _first, iterator _last) {
			if (_first != _last && _last != _pos && _first != _pos) {
				__transfer_(_pos, _first, _last);
			}
		}

		//merge:合并两个有序链表,最后会合并至第一链表处
		template<class Comp = LT::less<T>>
		void merge(list& _ls, Comp _cmp = Comp()) {
			if (_ls == *this) { return; }

			iterator beg1 = begin();
			iterator end1 = end();
			iterator beg2 = _ls.begin();
			iterator end2 = _ls.end();

			//注意list迭代器没有定义大于小于这些比较符
			while (beg1 != end1 && beg2 != end2) {
				if (_cmp(*beg1, *beg2)) {
					++beg1;
				}
				else {
					//说明需要将2合并过去了。
					iterator tmpBeg2 = beg2;
					while (beg2 != end2 && _cmp(*beg2, *beg1)) {
						++beg2;
					}
					__transfer_(beg1, tmpBeg2, beg2);
				}
				if (beg2 != end2) {
					__transfer_(end1, beg2, end2);
				}
			}
		}

		//反转链表
		void reverse() 
		{
			if ( nodePtr_->next == nodePtr_->pre) { return; }//size() == 0或size == 1。
			iterator beg = begin();
			while (beg != end()) {
				LT::swap(beg.nodePtr_->next, beg.nodePtr_->pre);
				--beg;
			}
			LT::swap(end().nodePtr_->next, end().nodePtr_->pre);
		}

		//链表排序
		template<class Comp = LT::less<T>>
		void sort(Comp _cmp = Comp()) 
		{
			__sort(begin(), size(), _cmp);
		}
		//***************************************************************************************************************************
		//***************************************************内部实现****************************************************************
		//***************************************************************************************************************************
		private:
		//---------------------------------------------------空间配置----------------------------------------------------------------
		typedef allocator<T> data_allocator;
		typedef allocator<list_node> node_allocator;
	
		//获得一个节点的空间，返回该节点指向该节点的指针
		inline list_node_pointer __allocate_one_node_mem() {
			list_node_pointer node = node_allocator::allocate();
			return node;
		}
		//析构一个节点的空间
		inline void __deallocate_one_node_mem(list_node_pointer _ptr) {
			node_allocator::deallocate(_ptr);
		}

		inline list_node_pointer __construct_one_node(list_node_pointer _ptr,
			const value_type& _value = T(), list_node_pointer _preNode = nullptr, list_node_pointer _nextNode = nullptr) {
			LT::construct(LT::address_of(_ptr->data), _value);
			_ptr->pre = _preNode;
			_ptr->next = _nextNode;
			return _ptr;
		}

		inline list_node_pointer __construct_one_node(list_node_pointer _ptr,
			value_type&& _value, list_node_pointer _preNode = nullptr, list_node_pointer _nextNode = nullptr) {
			LT::construct(LT::address_of(_ptr->data), LT::move(_value));
			_ptr->pre = _preNode;
			_ptr->next = _nextNode;
			return _ptr;
		}

		inline void __destroy_one_node(list_node_pointer _ptr) {
			LT::destroy(LT::address_of(_ptr->data));
		}

		//设置尾结点。
		inline void __set_tail() {
			//分配一个节点即可;
			nodePtr_ = __allocate_one_node_mem();//可以不初始化这里的data;
			nodePtr_->next = nodePtr_;
			nodePtr_->pre = nodePtr_;
		}
		//--------------------------------------------初始化函数----------------------------------------------------
		inline void __init() {
			__set_tail();
		}

		inline void __init_n(size_type _n, const value_type& _value) {
			nodePtr_ = __allocate_one_node_mem();
			list_node_pointer pre = nodePtr_;
			list_node_pointer cur = nodePtr_;
			for (; _n; --_n) {
				cur = __allocate_one_node_mem();
				LT::construct(cur, _value);
				pre->next = cur;
				cur->pre = pre;
				pre = cur;
			}
			nodePtr_->pre = cur;
			cur->next = nodePtr_;
		}

		template<class InputIter>
		inline void __init_iter(InputIter _itBeg, InputIter _itEnd) {
			__set_tail();
			size_type n = LT::distance(_itBeg, _itEnd);
			try {
				for (; n; --n, ++_itBeg) {
					insert(end(), _itBeg, _itEnd);
				}
			}
			catch(...){
				clear();
				__destroy_one_node(nodePtr_);
				__deallocate_one_node_mem(nodePtr_);
				nodePtr_ = nullptr;
			}
		}

		//析构的实现
		inline void __uninit() {
			list_node_pointer cur = nodePtr_;
			cur = cur->next;
			std::cout << size() << std::endl;
			while (cur != nodePtr_) {
				list_node_pointer tmp = cur;
				cur = cur->next;
				__destroy_one_node(tmp);
				__deallocate_one_node_mem(tmp);
			}
			__deallocate_one_node_mem(nodePtr_);
			nodePtr_ = nullptr;
		}

		//赋值assign的实现
		inline void __assignN(size_type _n, const value_type& _value) {
			iterator cur = begin();
			while (cur != end() && _n) {
				*cur = _value;
				--_n;
			}
			//如果空间过剩
			while (cur != end()) {
				cur = erase(cur,end());
			}
			while (_n) {
					insert(end(), _value);
					--_n;
			}	
		}

		
		template<class InputIterator>
		inline void __assign_iter(InputIterator _first, InputIterator _last) {
			/*size_type n = static_cast<size_type>(LT::distance(_first, _last));
			iterator cur = begin();
			while (cur != end() && n) {
				*_first = *cur;
				--n;
				++_first;
			}
			if (cur != end()) {
				 erase(cur, end());
			}
			else if (n) {
				insert(end(),n,*_first);
				++_first;
			}*/
		}

		//resize的实现
		inline void __resize(size_type _n, const value_type& _value) {
			iterator it = begin();
			for (; it != end() && _n; ++it, --_n) {}
			if (it != end()) { erase(it, end()); }
			while (_n)
			{
				emplace_back(_value);
			}
		}
		
		//emplace的实现
		template<class ...Args>
		inline void __emplace(iterator _pos, Args&& ..._value) {
			list_node_pointer pre = _pos.nodePtr_->pre;
			//list_node_pointer newNode = 
		}
		//-------------------------------------------内部接口---------------------------------------------------------
		void __transfer_(iterator _pos, iterator _first, iterator _last) {//将[first,end)之间的元素移到_pos之前。可以是同一个list内的两段区间
			if (_pos != _last) {
				list_node_pointer preNode = _pos.nodePtr_->pre;
				list_node_pointer tailNode = _last.nodePtr_->pre;

				_first.nodePtr_->pre->next = _last.nodePtr_;
				_last.nodePtr_->pre = _first.nodePtr_->pre;

				_first.nodePtr_->pre = preNode;
				preNode->next = _first.nodePtr_;
				tailNode->next = _pos.nodePtr_;
				_pos.nodePtr_->pre = tailNode;
			}
		}

		//排序
		template<class Comp>
		void __sort(iterator _itBegin, size_type _n, Comp _cmp) {
			if (_n == 1 || _n == 0) { return; }

			//使用归并排序
			size_type half = _n / 2;
			iterator theNext = _itBegin + half;
			__sort(_itBegin, half, _cmp);
			__sort(theNext, _n - half, _cmp);
			//此时已经是两个有序链表了。
			
			while(_itBegin != _itBegin + half && theNext != _itBegin + _n) {
				if (_cmp(*_itBegin, *theNext)) {
					++_itBegin;
				}
				else {
				//说明需要将2合并过去了。
				iterator tmpBeg2 = _itBegin;
				while (theNext != _itBegin + _n && _cmp(*theNext, *_itBegin)) {
					++theNext;
				}
				__transfer_(_itBegin, tmpBeg2, theNext);
				}
				if (theNext != _itBegin + _n) {
					__transfer_(_itBegin + half, theNext, _itBegin + _n);
				}
			}

		}
	};
	
	//******************************************外部的重载函数****************************************
	template<class T>
	bool operator<(const list<T>& _lhs, const list<T>& _rhs) {
		return LT::lexicographical_compare(_lhs.cbegin(), _lhs.cend(), _rhs.cbegin(), _rhs.cend());
	}

	template<class T>
	bool operator>(const list<T>& _lhs, const list<T>& _rhs) {
		return LT::lexicographical_compare(_rhs.cbegin(), _rhs.cend(), _lhs.cbegin(), _lhs.cend());
	}
	template<class T>
	bool operator==(const list<T>& _lhs, const list<T>& _rhs) {
		auto it1 = _lhs.cbegin();
		auto it2 = _rhs.cbegin();
		while (it1 != _lhs.cend() && it2 != _rhs.cend())
		{
			if (*it1 != *it2) { return false; }
		}
		return it1 == _lhs.cend() && it2 = _rhs.cend();
	}
	template <class T>
	bool operator!=(const list<T>& _lhs, const list<T>& _rhs) {
		return !(_lhs == _rhs);
	}

	template <class T>
	bool operator<=(const list<T>& _lhs, const list<T>& _rhs)
	{
		return !(_rhs < _lhs);
	}

	template <class T>
	bool operator>=(const list<T>& _lhs, const list<T>& _rhs)
	{
		return !(_lhs < _rhs);
	}

	// swap
	template <class T>
	void swap(list<T>& _lhs, list<T>& _rhs) noexcept
	{
		_lhs.swap(_rhs);
	}
}