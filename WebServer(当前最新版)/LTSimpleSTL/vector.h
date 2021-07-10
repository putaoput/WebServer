//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

#include <assert.h>
#include "memory.h"
#include "iterator.h"
#include "type_traits.h"
#include "allocator.h"
#include "uninitialized.h"
#include "algobase.h"

//该文件负责提供vector，编写小技巧，不清楚参数的时候，新建一个std::vector,写出需要了解的方法，然后去看。
//!!!vector和string的初始化策略不同，vector如果使用拷贝初始化，那么初始化之后的容量将等于待拷贝的元素数量。
namespace LT {
	template<typename T, class Alloc = allocator<T>>
	class vector {
		//先定义一些类型
	public:
		typedef T											value_type;
		typedef value_type*									pointer;
		typedef const T* const_pointer;
		typedef T* iterator;
		typedef const T* const_iterator;
		typedef LT::reverse_iterator<iterator>				reverse_iterator;
		typedef LT::reverse_iterator<const_iterator>		const_reverse_iterator;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t										size_type;
		typedef ptrdiff_t									difference_type;

		//定义vector内部的三个重要的指针
	private:
		iterator start_;
		iterator finish_;
		iterator endOfStorage_;

		//定义一些参数
		constexpr static double EXPAN = 2.0; //扩容系数

/////********************************************************************************************************
//**************************************************对外接口**************************************************
////**********************************************************************************************************
	public:
		//写一些构造函数
		vector() :start_(0), finish_(0), endOfStorage_(0) {}
		explicit vector(const size_type _n)
			:start_(0), finish_(0), endOfStorage_(0)
		{
			__init_n(_n, T());
		}
		vector(const size_type _n, const value_type& _value)
			:start_(0), finish_(0), endOfStorage_(0)
		{
			__init_n(_n, _value);
		}
		template<typename InputIter,
			typename LT::enable_if<is_input_iterator<InputIter>::value, int>::type = 0>
		vector(InputIter _first, InputIter _last)
			: start_(0), finish_(0), endOfStorage_(0)
		{
			__init_iter(_first, _last);
		}
		vector(const vector& _v)
			:start_(0), finish_(0), endOfStorage_(0)
		{
			__init_iter(_v.cbegin(), _v.cend());
		}
		vector(vector&& _v)
			:start_(_v.start_),finish_(_v.finish_), endOfStorage_(_v.endOfStorage_)
		{
			//因为vector都是从堆上申请空间的，所以可以直接交换指针
			_v.start_ = static_cast<iterator>(nullptr);
			_v.finish_ = static_cast<iterator>(nullptr);
			_v.endOfStorage_ = static_cast<iterator>(nullptr);
		}

		//初始化列表的构造函数
		vector(std::initializer_list<value_type> _ilist)
			: start_(0), finish_(0), endOfStorage_(0)
		{
			__init_iter(_ilist.begin(), _ilist.end());
		}

		vector& operator=(const vector& _rhs) 
		{
			if (&_rhs != this) 
			{
				__init_iter(_rhs.cbegin(), _rhs.cend());//为什么只能用cbegin 和 cend。
			}
			
			return *this;
		}
		vector& operator=(vector&& _v) {
			//因为vector都是从堆上申请空间的，所以可以直接交换指针
			start_ = _v.start_;
			finish_ = _v.finish_;
			endOfStorage_ = _v.endOfStorage_;

			_v.start_ = static_cast<iterator>(nullptr);
			_v.finish_ = static_cast<iterator>(nullptr);
			_v.endOfStorage_ = static_cast<iterator>(nullptr);

			return *this;
		}
		//析构函数
		~vector() {
			//析构函数需要一一析构对象，然后返还分配的内存
			__destroy_vector(start_, finish_, endOfStorage_);
			start_ = finish_ = endOfStorage_ = static_cast<iterator>(nullptr);
		}
		//------------------------------------------------------------这一组是公共接口，对外的api----------------------------------------------
	public:
		//--------------------------------------------迭代器相关-------------------------------------
		iterator begin() { return start_; }
		const_iterator cbegin() const { return start_; }
		iterator end() { return finish_; }
		const_iterator cend() const { return finish_; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

		reference front() { return *start_; }
		reference back() { return *(finish_ - 1); }
		const_reference front() const { return *start_; }
		const_reference back() const { return *(finish_ - 1); }
		pointer data() { return static_cast<pointer>(start_); }//返回一个迭代器对应的指针
		const_pointer data()const { return static_cast<pointer>(start_); }

		//---------------------------------------------容量相关----------------------------------------
		size_type size() { return size_type(finish_ - start_); }
		size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T); }
		size_type capacity()const { return size_type(endOfStorage_ - start_); }
		bool empty()const { return start_ == finish_; }
		void resize(size_type _newSize, const T& _value) { __resize(_newSize, _value); }//改变size，但是不减小容量.保证resize()之后，如果扩容那么容量为_newSize
		void resize(size_type _newSize) { __resize(_newSize, T()); }
		void reserve(size_type _newCap) { __reserve(_newCap); }//只增大容量，如果扩容，那么容量为_newCap。
		void shrink_to_fit()//放弃多余容量
		{
			if (size() != capacity())
			{
				__move_house(size(), size());
			}
		}
		void clear()
		{
			__destroy_mem(start_, finish_);
			finish_ = start_;
		};

		//-------------------------------单个元素变更操作相关--------------------------------
		reference operator[](size_type _n) 
		{ 
			assert(_n < size());
			return *(start_ + _n);
		}
		const_reference operator[](size_type _n)const
		{
			assert(_n < size());
			return *(start_ + _n);
		}
		reference at(size_type _n) 
		{ 
			assert(_n < size());
			return *(start_ + _n);
		}
		const_reference at(size_type _n)const
		{
			assert(_n < size());
			return *(start_ + _n);
		}

		void pop_back() 
		{ 
			assert(finish_ != start_);
			LT::destroy(LT::address_of(*finish_--));
						
		}
		void push_back(const T& _value) {
			if (finish_ == endOfStorage_) {
				//说明空间不够，自动进行扩容
				__move_bigger_house(capacity() + 1);
			}
			__construct_mem_n(finish_++, 1, _value);
		}
		void push_back(const T&& _value)
		{
			emplace_back(LT::move(_value));
		}

		iterator insert(const_iterator _pos, const value_type& _value)
		{
			assert(_pos >= begin() && _pos <= end());
			size_type needCap = size() + 1;
			iterator pos = const_cast<iterator>(_pos);

			if (capacity() >= needCap)
			{
				auto p = __move_mem_back(_pos, 1);
				if (p.first)
				{
					*pos = _value;
				}
				else if (p.second)
				{
					__construct_mem_n(pos, 1, _value);
				}
				
			}
			else {
				pos = __reserve_midn(__get_new_memSize(needCap), static_cast<size_type>(_pos - start_), static_cast<size_type>(_pos - start_), 1);
				__construct_mem_n(pos,1, _value);
			}
			return pos;
		}
		iterator insert(const_iterator _pos, size_type _n, const value_type& _value)
		{
			assert(_pos >= cbegin() && _pos <= cend());
			size_type needCap = size() + _n;
			iterator pos = const_cast<iterator>(_pos);

			if (capacity() >= needCap)
			{
				auto res = __move_mem_back(pos, _n);
				size_type beenInitMemSize = res.first;
				size_type unInitMemSize = res.second;

				if (beenInitMemSize)
				{
					LT::fill_n(pos, beenInitMemSize, _value);
				}
				if (unInitMemSize)
				{
					LT::uninitialized_fill_n(finish_, unInitMemSize, _value);
				}
				
			}
			else {
				pos = __reserve_midn(__get_new_memSize(needCap),static_cast<size_type>(_pos - start_), 1);
				__construct_mem_n(pos, _n, _value);
			}
			return pos;
					
		}
		iterator insert(const_iterator _pos, value_type&& _value)
		{
			
			return emplace(_pos, LT::move(_value));
		}
		template <class InputIter,
				 typename LT::enable_if<is_input_iterator<InputIter>::value, int>::type = 0>
		iterator insert(const_iterator _pos, InputIter _first, InputIter _last) 
		{ 
			assert(_pos >= begin() && _pos <= end());
			size_type inputSize = LT::distance(_first, _last);
			size_type needCap = size() + inputSize;
			iterator pos = const_cast<iterator>(_pos);
			if (capacity() >= needCap)
			{
				auto res = __move_mem_back(pos, inputSize);
				size_type beenInitMemSize = res.first;
				size_type unInitMemSize = res.second;
				InputIter posMidEnd = _first;
				LT::advance(posMidEnd, beenInitMemSize);
				if (beenInitMemSize)
				{
					LT::uninitialized_copy(_first, posMidEnd, pos);
				}
				if (unInitMemSize)
				{
					LT::uninitialized_copy(posMidEnd, _last, finish_);
				}

			}
			else {
				pos = __reserve_midn(__get_new_memSize(needCap),static_cast<size_type>(_pos - start_), 1);
				LT::uninitialized_copy(_first, _last, pos);
			}

			return pos;
		}

		template <class ...Args>
		iterator emplace(const_iterator _pos, Args&& ..._args) 
		{
			return __emplace(_pos, LT::forward<Args>(_args)...);
		}
		template<class... Args>//c++14中将无返回值改成了返回一个引用.c++11之后才有可变参数模板
		reference emplace_back(Args&&... _value)
		{
			return *__emplace(end(), LT::forward<Args>(_value)...);
		}

		iterator erase(const_iterator _pos) {
			assert(_pos >= start_ && _pos < finish_);
			iterator eraseBeg = start_ + (_pos - cbegin());
			LT::move(eraseBeg + 1, finish_, eraseBeg);
			--finish_;
			return eraseBeg;
		}
		iterator erase(const_iterator _itBeg, const_iterator _itEnd)
		{
			assert(_itBeg >= start_ && _itEnd < finish_);
			size_type preSize = _itBeg - cbegin();
			iterator itBeg = start_ + preSize;
			iterator itEnd = itBeg + (_itEnd - _itBeg);
			iterator itEraseBeg = LT::move(itEnd, finish_, itBeg);
			__destroy_mem(itEraseBeg, end());
			finish_ = itEraseBeg;

			return itEraseBeg;
		}
		void swap(vector<int>& _v) {
			//进行swap之前注意判同
			if (this != &_v)
			{
				LT::swap(start_, _v.start_);
				LT::swap(finish_, _v.finish_);
				LT::swap(endOfStorage_, _v.endOfStorage_);
			}

		}

		//--------------------------------多个元素变更操作--------------------------------------------
		//assign函数相当于赋值初始化。如果原来容量比需要新拷贝的容量大，那么容量不变，只变化size。如果原有容量不够，将分配新内存空间。
		void assign(size_type _size, const T& _value)
		{
			size_type oldCap = capacity();
			if (oldCap >= _value) {
				__deallocate_mem(start_, endOfStorage_);
				__construct_mem_n(start_, _size, _value);
				finish_ = start_ + _size;
			}
			else {
				__destroy_vector(start_, finish_, endOfStorage_);
				__init_n(_value, _size);
			}
		}
		template <class InputIter, typename LT::enable_if<is_input_iterator<InputIter>::value, int>::type = 0>
		void assign(InputIter _first, InputIter _last)
		{
			size_type inputSize = LT::distance(_first, _last);
			size_type oldCap = capacity();
			if (oldCap >= inputSize) {
				__destroy_mem(start_, endOfStorage_);
				__construct_mem_iter(start_, _first, _last);
				finish_ = start_ + inputSize;
			}
			else {
				__destroy_vector(start_, finish_, endOfStorage_);
				__init_iter(_first, _last);
			}
		}
		void assign(std::initializer_list<value_type> _il)
		{
			assign(_il.begin(), _il.end());
		}

		//reverse:
		void reverse()
		{
			iterator beg = start_;
			iterator end = finish_;
			while (beg != end)
			{
				LT::iter_swap(beg, end);
				++beg;
				--end;
			}
		}
		//--------------------------------------------------------------容量配置器相关-----------------------------------------
		Alloc get_allocator() const { return allocator_type(); }
		/////********************************************************************************************************
		//**************************************************内部实现**************************************************
		////**********************************************************************************************************
	private:
	//----------------------------------------------声明一组函数用来配置空间，包括进行初始化------------------------
	//要完成以下内容:
		//1.配置一块大小为n的内存，并对齐进行初始化
		//2.配置一块大小为n的内存，对其中大小为m的部分进行拷贝初始化，剩下的内存进行
		//3.鉴于此，分为两部分，一部分是获得给定大小的内存，一部分是初始化给定大小的内存区域

	//空间配置函数
		typedef Alloc            allocator_type;

		//获得给定大小的内存区域,在这里有异常保证
		iterator __get_mem(size_type _size)
		{
			//事实上allo函数返回的是T*。但是不保证该内存大小一定为_size，
			//所以调用该函数的之前，一定要保证_size是8的整数倍(因为内存池的原因)
			try {
				pointer memPtr = allocator_type::allocate(_size);
				return static_cast<iterator>(memPtr);//vector实现时将T*定义成了iterator。
			}
			catch (...) {//这一层异常保证也许可以省略，因为在alloctor文件里面应该是提供了异常保证的
				return static_cast<iterator>(nullptr);
			}

		}

		//可进行右值初始化的函数
		template<class ...Args>
		void __construct_one(iterator _pos, Args&&... _args)
		{
			LT::construct(LT::address_of(*_pos), LT::forward<Args>(_args)...);
		}
		//初始化给定大小的区域。进行异常保证，这是左值构造
		void __construct_mem_n(iterator _begin, size_type _size, const value_type& _value) {
			LT::uninitialized_fill_n(_begin, _size, _value);
		}
		//这里要保证_begin的大小足够，顺序复制
		template<class InputIter>		
		void __construct_mem_iter(iterator _begin, InputIter _itBeg, InputIter _itEnd)
		{
			size_type n = LT::distance(_itBeg, _itEnd);
			LT::uninitialized_copy(_itBeg, _itEnd, _begin);
		}

		//析构对象的函数
		void __destroy_mem(iterator _first, iterator _last)
		{
			//下一层construct函数会自己进行类型萃取，识别是否需要调用析构函数
			LT::destroy(_first, _last);
		}
		//释放内存
		template<class iterator>
		void __deallocate_mem(iterator _first, iterator _endOfStorage)
		{
			allocator_type::deallocate(_first, static_cast<size_type>((_endOfStorage - start_)));
		}
		//实际进行vector析构的函数
		void __destroy_vector(iterator _start, iterator _finish, iterator _endOfStorage) {
			__destroy_mem(_start, _finish);
			__deallocate_mem(_start, _endOfStorage);
		}

		//用于确定扩容是需要的新容量大小
		size_type __get_new_memSize(size_type _needSize)
		{
			size_type initCap = max(static_cast<size_type>(1), capacity());
			while (initCap < _needSize && static_cast<size_type>(initCap * EXPAN) == initCap)//防止出现由于EXPAN系数小而无法扩容的bug
			{
				++initCap;
			}
			while (initCap < _needSize)
			{
				initCap *= EXPAN;
				
			}
			return initCap;
		}

		//-------------------------------------------初始化函数----------------------------------------------------------------------
		//进行一个给定初始size的初始化。
		void __init_n(size_type _size, const value_type& _value) {

			iterator newMem = __get_mem(_size);
			__construct_mem_n(newMem, _size, _value);
			start_ = newMem;
			finish_ = newMem + _size;
			endOfStorage_ = newMem + _size;
		}
		//提供输入一组迭代器的_init版本
		template<typename InputIterator>
		void __init_iter(InputIterator _first, InputIterator _last) 
		{

			//先确定长度
			int needSize = LT::distance(_first, _last);
			if (needSize <= 0) { return; }

			//然后申请空间
			iterator newMem = __get_mem(needSize);

			LT::uninitialized_copy(_first, _last, newMem);
			start_ = newMem;
			finish_ = newMem + needSize;
			endOfStorage_ = newMem + needSize;
		}	
		//如果输入的一组迭代器可以确定是在一片连续内存上，那么可以使用特化版本
		template<>
		void __init_iter<const_iterator>(const_iterator _first, const_iterator _last)
		{
			//先确定长度
			int needSize = LT::distance(_first, _last);
			if (needSize <= 0) { return; }

			//然后申请空间
			iterator newMem = __get_mem(needSize);
			LT::uninitialized_copy(_first, _last, newMem);
			
			start_ = newMem;
			finish_ = newMem + needSize;
			endOfStorage_ = newMem + needSize;
		}

		//提供输入一组迭代器的,并且给定容量的_init版本,如果给定的容量小于输入迭代器的大小，将会只拷贝给定容量,
		//如果给_copySize大于那组输入迭代器所管理区域的元素的数量，额外的元素将被初始化给定的值_extrVal
		//所以该函数保证分配的容量为_n。使用了此版本的__init(),将可能不会复制所有待拷贝对象(_n < copySize)。
		//该函数至少会初始化n个对象，其中有min(_copySize,_last - _first)个是拷贝初始化。
		//但是调用此版本除了reverse,都是_n > copySize。
		//该函数写的不是很好
		template<typename InputIterator>
		void __init_iter_n(size_type _n ,InputIterator _first, InputIterator _last,
			iterator _start, iterator _finish, iterator _endOfStorage, 
			size_type _copySize = 0, const T& _extrVal = T())
		{	
			//先确定长度
			if (!_copySize) { _copySize = LT::distance(_first, _last); }
			size_type copySize = min(_copySize, static_cast<size_type>(_last - _first));
			size_type needSize = _n;
			size_type extrConsSize = copySize - static_cast<size_type>(_last - _first);
			if (needSize <= 0) { return; }

			//然后申请空间
			size_type realSize = (needSize / 8) * 8;
			if (needSize % 8) { realSize += 8; }
			iterator newMem = __get_mem(realSize * sizeof(value_type));

			//一个一个的填充
			if (newMem == static_cast<iterator>(nullptr)) {
				//分配失败了。什么也不做.
				return;
			}
			else {
				//先进行拷贝填充
				for (int i = 0; i < copySize; ++i) {
					__construct_mem_n(newMem + i, 1, *(_first + i));
				}
				//再进行给定值拷贝初始化
				if (extrConsSize > 0) {
					for (int i = copySize; i < copySize + extrConsSize; ++i) {
						__construct_mem_n(newMem + i, 1, _extrVal);
					}
				}
				_start = newMem;
				_finish = extrConsSize > 0 ? newMem + needSize + extrConsSize : newMem + needSize;
				_endOfStorage = newMem + realSize;
			}
		}

		//----------------------------------------扩容,变容函数-------------------------------------------
		//自动获得一片新的大小为_newSize内存区域，并且将原区域的_moveSize个元素移动到新的区域
		void __move_house(size_type _newSize, size_type _moveSize = size())
		{
			assert(_newSize >= _moveSize);
			iterator newStart = __get_mem(_newSize);
			LT::uninitialized_move(start_, start_ + _moveSize, newStart);

			//析构当前内存区域
			__destroy_vector(start_, finish_, endOfStorage_);
			//指向新的内存区域
			start_ = newStart;
			finish_ = newStart + _moveSize;
			endOfStorage_ = newStart + _newSize;
		}
		//该函数负责在现有容量不足时，扩容并且搬移到新内存，输入参数为需要容纳的元素大小
		void __move_bigger_house(size_type _needSize) {
			size_type oldCap = capacity();
			size_type newCap = oldCap;
			while (newCap < _needSize)
			{
				newCap *= EXPAN;
			}
			if (newCap != oldCap)
			{
				__move_house(newCap, size());
			}
		}
		
		//-------------------------------------内存移动函数-----------------------------------------------
		//把从_pos开始的元素在内存区域内向后移动n个距离,调用该函数的前提是空间足够大
		LT::pair<size_type, size_type> __move_mem_back(iterator _pos, size_type _n)
		{
			if (_pos == finish_) 
			{ 
				finish_ += _n;
				return make_pair<size_type,size_type>(0,0);
			}
			size_type backSize = finish_ - _pos;

			//如果n比backSize小，那么是需要把n个元素移动到未初始化的内存区域
			//如果n比backSize大，那么是需要把backSize个元素移动到未初始化的区域，其中有backSize个元素将需要被拷贝初始化	
			if (_n < backSize)
			{
				LT::uninitialized_move(finish_ - _n, finish_, finish_);//后面_n个是复制到未初始化空间
				size_type remainToCopy = backSize - _n;//此时还有backSize - _n个是复制到了_pos + _n,finish_，这个区间
				LT::move_backward(_pos, _pos + remainToCopy, finish_);
			}
			else
			{
				//此时全部都拷贝到未初始化空间
				LT::uninitialized_copy(_pos, finish_, _pos + _n);
			}
			size_type beenInitMemSize = min(_n, backSize);
			size_type unInitMemSize = _n > backSize ? _n - backSize : 0;

			finish_ += _n;
			return LT::make_pair<size_type, size_type>(beenInitMemSize, unInitMemSize);
		}
		//把从_pos开始的元素在内存区域内向前移动n个距离
		void __move_mem_forward(iterator _pos, size_type _n)
		{
			size_type startPos = _pos - _n;
			assert(startPos < start_);
			LT::move(_pos, finish_, startPos);

		}
		void __resize(size_type _newSize, value_type _value)
		{
			//要进行异常保证，所以构造的时候要不就是一起构造，要不就是一起不构造。
			//使用提供异常保证的内存管理工具，在"uninitialized.h"头文件上。
			if(_newSize < size()){
				//缩减size，但是不缩减容量
				__destroy_mem(start_ + _newSize, start_ + size());
				finish_ = start_ + _newSize;
			}
			if (_newSize > size()) {
				__reserve(_newSize);
				LT::uninitialized_fill(finish_, endOfStorage_, _value);
			}
		}
		void __reserve(size_type _newSize) 
		{
			int curCap = capacity();
			if (_newSize < capacity()) {
				__move_house(_newSize, size());
			}
		}
		iterator __reserve_midn(size_type _newCap, size_type _pos, size_type _n)
		{
			//该函数会进行扩容,但是扩容之后的容量就是_newCap，与reserver不同的是，它会在中间第pos个位置开始，留出_n个未初始化的空间
			//调用该函数的前提就是当前的容量小于新容量,但是这里为了性能不做断言。
			//该函数会设置好新的start_, finish_,endOfStorage
			pointer newMem = __get_mem(_newCap);
			size_type oldSize = size();
			assert(_pos <= oldSize);
			LT::uninitialized_move(start_, start_ + _pos, newMem);
			LT::uninitialized_move( start_ + _pos, finish_ ,newMem + _pos + _n);
			__destroy_mem(start_, finish_);
			__deallocate_mem(start_, endOfStorage_);
			start_ = newMem;
			finish_ = start_ + oldSize + _n;
			endOfStorage_ = start_ + _newCap;
			return _pos + start_;
		}
		
		//------------------------------------单个元素操作-------------------------------------------
		//负责右值拷贝或是左值拷贝的方式在一个位置插入若干元素。
		template <class ...Args>//传入的参数包可能是初始化参数列表
		inline iterator __emplace(const_iterator _pos, Args&& ..._args)
		{
			assert(_pos >= begin() && _pos <= end());
			size_type needCap = size() + 1;
			iterator pos = const_cast<iterator>(_pos);

			if (capacity() >= needCap)
			{
				auto p = __move_mem_back(pos, 1);
				LT::construct(LT::address_of(*pos), LT::forward<Args>(_args)...);
			}
			else {
				pos = __reserve_midn(__get_new_memSize(needCap), static_cast<size_type>(_pos - start_), 1);
				LT::construct(LT::address_of(*pos), LT::forward<Args>(_args)...);
			}
			return pos;
		}
	};

	//***************************************************************************************************
	//***************************************对一部分函数进行外部重载************************************
	//***************************************************************************************************
	//重载swap
	template<class T>
	void swap(vector<T>& _lhs, vector<T>& _rhs) {
		_lhs.swap(_rhs);
	}

	//------------------------------------重载比较操作符-------------------------------------------------
	template<class T>
	bool operator==(const vector<T> _lhs, const vector<T>& _rhs) {
		return _lhs.size() == _rhs.size() && LT::equal_to(_lhs.begin(), _lhs.end(), _rhs.begin());
	}
	template<class T>
	bool operator!=(const vector<T> _lhs, const vector<T>& _rhs) {
		return !(_lhs == _rhs.size());
	}
	template<class T>
	bool operator<(const vector<T> _lhs, const vector<T>& _rhs) {
		return LT::lexicographical_compare(_lhs.cbegin(), _lhs.cend(), _rhs.cbegin(), _rhs.cend());
	}
	template<class T>
	bool operator<=(const vector<T> _lhs, const vector<T>& _rhs) {
		return !_lhs > _rhs;
	}
	template<class T>
	bool operator>(const vector<T> _lhs, const vector<T>& _rhs) {
		return LT::lexicographical_compare(_rhs.cbegin(), _rhs.cend(), _lhs.cbegin(), _lhs.cend());
	}
	template<class T>
	bool operator>=(const vector<T> _lhs, const vector<T>& _rhs) {
		return !_lhs < _rhs;
	}
}