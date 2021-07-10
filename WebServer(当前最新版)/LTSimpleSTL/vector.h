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

//���ļ������ṩvector����дС���ɣ������������ʱ���½�һ��std::vector,д����Ҫ�˽�ķ�����Ȼ��ȥ����
//!!!vector��string�ĳ�ʼ�����Բ�ͬ��vector���ʹ�ÿ�����ʼ������ô��ʼ��֮������������ڴ�������Ԫ��������
namespace LT {
	template<typename T, class Alloc = allocator<T>>
	class vector {
		//�ȶ���һЩ����
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

		//����vector�ڲ���������Ҫ��ָ��
	private:
		iterator start_;
		iterator finish_;
		iterator endOfStorage_;

		//����һЩ����
		constexpr static double EXPAN = 2.0; //����ϵ��

/////********************************************************************************************************
//**************************************************����ӿ�**************************************************
////**********************************************************************************************************
	public:
		//дһЩ���캯��
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
			//��Ϊvector���ǴӶ�������ռ�ģ����Կ���ֱ�ӽ���ָ��
			_v.start_ = static_cast<iterator>(nullptr);
			_v.finish_ = static_cast<iterator>(nullptr);
			_v.endOfStorage_ = static_cast<iterator>(nullptr);
		}

		//��ʼ���б�Ĺ��캯��
		vector(std::initializer_list<value_type> _ilist)
			: start_(0), finish_(0), endOfStorage_(0)
		{
			__init_iter(_ilist.begin(), _ilist.end());
		}

		vector& operator=(const vector& _rhs) 
		{
			if (&_rhs != this) 
			{
				__init_iter(_rhs.cbegin(), _rhs.cend());//Ϊʲôֻ����cbegin �� cend��
			}
			
			return *this;
		}
		vector& operator=(vector&& _v) {
			//��Ϊvector���ǴӶ�������ռ�ģ����Կ���ֱ�ӽ���ָ��
			start_ = _v.start_;
			finish_ = _v.finish_;
			endOfStorage_ = _v.endOfStorage_;

			_v.start_ = static_cast<iterator>(nullptr);
			_v.finish_ = static_cast<iterator>(nullptr);
			_v.endOfStorage_ = static_cast<iterator>(nullptr);

			return *this;
		}
		//��������
		~vector() {
			//����������Ҫһһ��������Ȼ�󷵻�������ڴ�
			__destroy_vector(start_, finish_, endOfStorage_);
			start_ = finish_ = endOfStorage_ = static_cast<iterator>(nullptr);
		}
		//------------------------------------------------------------��һ���ǹ����ӿڣ������api----------------------------------------------
	public:
		//--------------------------------------------���������-------------------------------------
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
		pointer data() { return static_cast<pointer>(start_); }//����һ����������Ӧ��ָ��
		const_pointer data()const { return static_cast<pointer>(start_); }

		//---------------------------------------------�������----------------------------------------
		size_type size() { return size_type(finish_ - start_); }
		size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T); }
		size_type capacity()const { return size_type(endOfStorage_ - start_); }
		bool empty()const { return start_ == finish_; }
		void resize(size_type _newSize, const T& _value) { __resize(_newSize, _value); }//�ı�size�����ǲ���С����.��֤resize()֮�����������ô����Ϊ_newSize
		void resize(size_type _newSize) { __resize(_newSize, T()); }
		void reserve(size_type _newCap) { __reserve(_newCap); }//ֻ����������������ݣ���ô����Ϊ_newCap��
		void shrink_to_fit()//������������
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

		//-------------------------------����Ԫ�ر���������--------------------------------
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
				//˵���ռ䲻�����Զ���������
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
		template<class... Args>//c++14�н��޷���ֵ�ĳ��˷���һ������.c++11֮����пɱ����ģ��
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
			//����swap֮ǰע����ͬ
			if (this != &_v)
			{
				LT::swap(start_, _v.start_);
				LT::swap(finish_, _v.finish_);
				LT::swap(endOfStorage_, _v.endOfStorage_);
			}

		}

		//--------------------------------���Ԫ�ر������--------------------------------------------
		//assign�����൱�ڸ�ֵ��ʼ�������ԭ����������Ҫ�¿�������������ô�������䣬ֻ�仯size�����ԭ���������������������ڴ�ռ䡣
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
		//--------------------------------------------------------------�������������-----------------------------------------
		Alloc get_allocator() const { return allocator_type(); }
		/////********************************************************************************************************
		//**************************************************�ڲ�ʵ��**************************************************
		////**********************************************************************************************************
	private:
	//----------------------------------------------����һ�麯���������ÿռ䣬�������г�ʼ��------------------------
	//Ҫ�����������:
		//1.����һ���СΪn���ڴ棬��������г�ʼ��
		//2.����һ���СΪn���ڴ棬�����д�СΪm�Ĳ��ֽ��п�����ʼ����ʣ�µ��ڴ����
		//3.���ڴˣ���Ϊ�����֣�һ�����ǻ�ø�����С���ڴ棬һ�����ǳ�ʼ��������С���ڴ�����

	//�ռ����ú���
		typedef Alloc            allocator_type;

		//��ø�����С���ڴ�����,���������쳣��֤
		iterator __get_mem(size_type _size)
		{
			//��ʵ��allo�������ص���T*�����ǲ���֤���ڴ��Сһ��Ϊ_size��
			//���Ե��øú�����֮ǰ��һ��Ҫ��֤_size��8��������(��Ϊ�ڴ�ص�ԭ��)
			try {
				pointer memPtr = allocator_type::allocate(_size);
				return static_cast<iterator>(memPtr);//vectorʵ��ʱ��T*�������iterator��
			}
			catch (...) {//��һ���쳣��֤Ҳ�����ʡ�ԣ���Ϊ��alloctor�ļ�����Ӧ�����ṩ���쳣��֤��
				return static_cast<iterator>(nullptr);
			}

		}

		//�ɽ�����ֵ��ʼ���ĺ���
		template<class ...Args>
		void __construct_one(iterator _pos, Args&&... _args)
		{
			LT::construct(LT::address_of(*_pos), LT::forward<Args>(_args)...);
		}
		//��ʼ��������С�����򡣽����쳣��֤��������ֵ����
		void __construct_mem_n(iterator _begin, size_type _size, const value_type& _value) {
			LT::uninitialized_fill_n(_begin, _size, _value);
		}
		//����Ҫ��֤_begin�Ĵ�С�㹻��˳����
		template<class InputIter>		
		void __construct_mem_iter(iterator _begin, InputIter _itBeg, InputIter _itEnd)
		{
			size_type n = LT::distance(_itBeg, _itEnd);
			LT::uninitialized_copy(_itBeg, _itEnd, _begin);
		}

		//��������ĺ���
		void __destroy_mem(iterator _first, iterator _last)
		{
			//��һ��construct�������Լ�����������ȡ��ʶ���Ƿ���Ҫ������������
			LT::destroy(_first, _last);
		}
		//�ͷ��ڴ�
		template<class iterator>
		void __deallocate_mem(iterator _first, iterator _endOfStorage)
		{
			allocator_type::deallocate(_first, static_cast<size_type>((_endOfStorage - start_)));
		}
		//ʵ�ʽ���vector�����ĺ���
		void __destroy_vector(iterator _start, iterator _finish, iterator _endOfStorage) {
			__destroy_mem(_start, _finish);
			__deallocate_mem(_start, _endOfStorage);
		}

		//����ȷ����������Ҫ����������С
		size_type __get_new_memSize(size_type _needSize)
		{
			size_type initCap = max(static_cast<size_type>(1), capacity());
			while (initCap < _needSize && static_cast<size_type>(initCap * EXPAN) == initCap)//��ֹ��������EXPANϵ��С���޷����ݵ�bug
			{
				++initCap;
			}
			while (initCap < _needSize)
			{
				initCap *= EXPAN;
				
			}
			return initCap;
		}

		//-------------------------------------------��ʼ������----------------------------------------------------------------------
		//����һ��������ʼsize�ĳ�ʼ����
		void __init_n(size_type _size, const value_type& _value) {

			iterator newMem = __get_mem(_size);
			__construct_mem_n(newMem, _size, _value);
			start_ = newMem;
			finish_ = newMem + _size;
			endOfStorage_ = newMem + _size;
		}
		//�ṩ����һ���������_init�汾
		template<typename InputIterator>
		void __init_iter(InputIterator _first, InputIterator _last) 
		{

			//��ȷ������
			int needSize = LT::distance(_first, _last);
			if (needSize <= 0) { return; }

			//Ȼ������ռ�
			iterator newMem = __get_mem(needSize);

			LT::uninitialized_copy(_first, _last, newMem);
			start_ = newMem;
			finish_ = newMem + needSize;
			endOfStorage_ = newMem + needSize;
		}	
		//��������һ�����������ȷ������һƬ�����ڴ��ϣ���ô����ʹ���ػ��汾
		template<>
		void __init_iter<const_iterator>(const_iterator _first, const_iterator _last)
		{
			//��ȷ������
			int needSize = LT::distance(_first, _last);
			if (needSize <= 0) { return; }

			//Ȼ������ռ�
			iterator newMem = __get_mem(needSize);
			LT::uninitialized_copy(_first, _last, newMem);
			
			start_ = newMem;
			finish_ = newMem + needSize;
			endOfStorage_ = newMem + needSize;
		}

		//�ṩ����һ���������,���Ҹ���������_init�汾,�������������С������������Ĵ�С������ֻ������������,
		//�����_copySize����������������������������Ԫ�ص������������Ԫ�ؽ�����ʼ��������ֵ_extrVal
		//���Ըú�����֤���������Ϊ_n��ʹ���˴˰汾��__init(),�����ܲ��Ḵ�����д���������(_n < copySize)��
		//�ú������ٻ��ʼ��n������������min(_copySize,_last - _first)���ǿ�����ʼ����
		//���ǵ��ô˰汾����reverse,����_n > copySize��
		//�ú���д�Ĳ��Ǻܺ�
		template<typename InputIterator>
		void __init_iter_n(size_type _n ,InputIterator _first, InputIterator _last,
			iterator _start, iterator _finish, iterator _endOfStorage, 
			size_type _copySize = 0, const T& _extrVal = T())
		{	
			//��ȷ������
			if (!_copySize) { _copySize = LT::distance(_first, _last); }
			size_type copySize = min(_copySize, static_cast<size_type>(_last - _first));
			size_type needSize = _n;
			size_type extrConsSize = copySize - static_cast<size_type>(_last - _first);
			if (needSize <= 0) { return; }

			//Ȼ������ռ�
			size_type realSize = (needSize / 8) * 8;
			if (needSize % 8) { realSize += 8; }
			iterator newMem = __get_mem(realSize * sizeof(value_type));

			//һ��һ�������
			if (newMem == static_cast<iterator>(nullptr)) {
				//����ʧ���ˡ�ʲôҲ����.
				return;
			}
			else {
				//�Ƚ��п������
				for (int i = 0; i < copySize; ++i) {
					__construct_mem_n(newMem + i, 1, *(_first + i));
				}
				//�ٽ��и���ֵ������ʼ��
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

		//----------------------------------------����,���ݺ���-------------------------------------------
		//�Զ����һƬ�µĴ�СΪ_newSize�ڴ����򣬲��ҽ�ԭ�����_moveSize��Ԫ���ƶ����µ�����
		void __move_house(size_type _newSize, size_type _moveSize = size())
		{
			assert(_newSize >= _moveSize);
			iterator newStart = __get_mem(_newSize);
			LT::uninitialized_move(start_, start_ + _moveSize, newStart);

			//������ǰ�ڴ�����
			__destroy_vector(start_, finish_, endOfStorage_);
			//ָ���µ��ڴ�����
			start_ = newStart;
			finish_ = newStart + _moveSize;
			endOfStorage_ = newStart + _newSize;
		}
		//�ú���������������������ʱ�����ݲ��Ұ��Ƶ����ڴ棬�������Ϊ��Ҫ���ɵ�Ԫ�ش�С
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
		
		//-------------------------------------�ڴ��ƶ�����-----------------------------------------------
		//�Ѵ�_pos��ʼ��Ԫ�����ڴ�����������ƶ�n������,���øú�����ǰ���ǿռ��㹻��
		LT::pair<size_type, size_type> __move_mem_back(iterator _pos, size_type _n)
		{
			if (_pos == finish_) 
			{ 
				finish_ += _n;
				return make_pair<size_type,size_type>(0,0);
			}
			size_type backSize = finish_ - _pos;

			//���n��backSizeС����ô����Ҫ��n��Ԫ���ƶ���δ��ʼ�����ڴ�����
			//���n��backSize����ô����Ҫ��backSize��Ԫ���ƶ���δ��ʼ��������������backSize��Ԫ�ؽ���Ҫ��������ʼ��	
			if (_n < backSize)
			{
				LT::uninitialized_move(finish_ - _n, finish_, finish_);//����_n���Ǹ��Ƶ�δ��ʼ���ռ�
				size_type remainToCopy = backSize - _n;//��ʱ����backSize - _n���Ǹ��Ƶ���_pos + _n,finish_���������
				LT::move_backward(_pos, _pos + remainToCopy, finish_);
			}
			else
			{
				//��ʱȫ����������δ��ʼ���ռ�
				LT::uninitialized_copy(_pos, finish_, _pos + _n);
			}
			size_type beenInitMemSize = min(_n, backSize);
			size_type unInitMemSize = _n > backSize ? _n - backSize : 0;

			finish_ += _n;
			return LT::make_pair<size_type, size_type>(beenInitMemSize, unInitMemSize);
		}
		//�Ѵ�_pos��ʼ��Ԫ�����ڴ���������ǰ�ƶ�n������
		void __move_mem_forward(iterator _pos, size_type _n)
		{
			size_type startPos = _pos - _n;
			assert(startPos < start_);
			LT::move(_pos, finish_, startPos);

		}
		void __resize(size_type _newSize, value_type _value)
		{
			//Ҫ�����쳣��֤�����Թ����ʱ��Ҫ������һ���죬Ҫ������һ�𲻹��졣
			//ʹ���ṩ�쳣��֤���ڴ�����ߣ���"uninitialized.h"ͷ�ļ��ϡ�
			if(_newSize < size()){
				//����size�����ǲ���������
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
			//�ú������������,��������֮�����������_newCap����reserver��ͬ���ǣ��������м��pos��λ�ÿ�ʼ������_n��δ��ʼ���Ŀռ�
			//���øú�����ǰ����ǵ�ǰ������С��������,��������Ϊ�����ܲ������ԡ�
			//�ú��������ú��µ�start_, finish_,endOfStorage
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
		
		//------------------------------------����Ԫ�ز���-------------------------------------------
		//������ֵ����������ֵ�����ķ�ʽ��һ��λ�ò�������Ԫ�ء�
		template <class ...Args>//����Ĳ����������ǳ�ʼ�������б�
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
	//***************************************��һ���ֺ��������ⲿ����************************************
	//***************************************************************************************************
	//����swap
	template<class T>
	void swap(vector<T>& _lhs, vector<T>& _rhs) {
		_lhs.swap(_rhs);
	}

	//------------------------------------���رȽϲ�����-------------------------------------------------
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