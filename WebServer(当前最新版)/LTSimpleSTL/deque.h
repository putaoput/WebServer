//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ������ṩdeque
#include "allocator.h"
#include "alloc.h"
#include "uninitialized.h"
#include "construct.h"
#include "algobase.h"
#include <assert.h>
#include "vector.h"

namespace LT {
	//*************************************************************************************************
	//*****************************************deque��buf*******************************************
	//*************************************************************************************************
	
	static size_t __deque_buf_size(size_t _bufSize, size_t _TSize) {
		size_t dequeBufSize =  _bufSize / _TSize;
		return dequeBufSize ? dequeBufSize : 1;//bufSize���ٿ�������һ��T��
	}
	

	//*************************************************************************************************
	//*****************************************deque�ĵ�����*******************************************
	//*************************************************************************************************

	//Ϊ����deque�ķֶ������ռ俴��������һ�����壬��Ҫ����һ������������Ҫ�ܿ����ģ���������++��--�������������
	template <class T, class Ref, class Ptr, size_t _BufSize>
	struct __deque_iterator : public iterator<random_access_iterator_tag, T>
	{
		typedef __deque_iterator<T, T&, T*, _BufSize>  iterator;
		typedef __deque_iterator<T, const T&, const T*, _BufSize>  const_iterator;
		static size_t buffer_size() { return __deque_buf_size(_BufSize, sizeof(T)); }

		////����׫д�����Ҫ�ĵ������ͱ�
		//typedef random_access_iterator_tag                          iterator_category;
		typedef T													value_type;
		typedef Ptr													pointer;
		typedef Ref													reference;
		typedef size_t												size_type;
		typedef ptrdiff_t											difference_type;
		typedef T*													value_pointer;//ֵָ��
		typedef T**													map_pointer;//����ָ��

		typedef __deque_iterator<T, T&, T*, _BufSize>				iterator;
		typedef __deque_iterator<T, const T&, const T*, _BufSize>	const_iterator;
		typedef __deque_iterator									self;


		//һЩָ����������������������
		pointer cur_;
		pointer first_;
		pointer last_;
		map_pointer node_; //ָ��ܿ�����
		static const size_type MIN_MAP_SIZE = 8;//ÿ��map���ٹ���8����㡣��Ϊ�Ǿ�̬���������Բ�����ÿ��deque����һ�ݡ�
		//----------------------------------------------���캯��----------------------------------------
		__deque_iterator():cur_(nullptr),first_(nullptr),last_(nullptr),node_(nullptr){}
		__deque_iterator(value_pointer _vp, map_pointer _mp) :cur_(_vp), first_(*_mp), last_(*_mp + buffer_size()), node_(_mp) {}
		__deque_iterator(const iterator& _rhs):cur_(_rhs.cur_), first_(_rhs.first_), last_(_rhs.last_), node_(_rhs.node_) {}
		__deque_iterator(iterator&& _rhs) :cur_(_rhs.cur_), first_(_rhs.first_), last_(_rhs.last_), node_(_rhs.node_)
		{
			_rhs.cur_ = nullptr; _rhs.first_ = nullptr; _rhs.last_ = nullptr; _rhs.node_ = nullptr;
		}
		__deque_iterator(const const_iterator& _rhs) :cur_(_rhs.cur_), first_(_rhs.first_), last_(_rhs.last_), node_(_rhs.node_) {}
		self& operator=(const iterator& _rhs) {
			if (*this != _rhs) { cur_ = _rhs.cur_; first_ = _rhs.first_; last_ = _rhs.last_; node_ = _rhs.node_; }
			return *this;
		}
		self& operator=(const const_iterator& _rhs)
		{
			if (*this != _rhs) { cur_ = _rhs.cur_; first_ = _rhs.first_; last_ = _rhs.last_; node_ = _rhs.node_; }
			return *this;
		}
		self& operator=(iterator&& _rhs)
		{
			node_ = _rhs.node_;
			cur_ = _rhs.cur_;
			first_ = _rhs.first_;
			last_ = _rhs.last_;
			_rhs.clear();
			return *this;
		}
		void set_node(map_pointer _newNode) {
			node_ = _newNode;
			first_ = *_newNode;
			last_ = first_ + buffer_size();
		}

		void clear() 
		{
			node_ = nullptr;
			cur_ = nullptr;
			first_ = nullptr;
			last_ = nullptr;
		}

		void swap(const iterator& _rhs)
		{
			LT::swap(_rhs.cur_, cur_);
			LT::swap(_rhs.first_, first_);
			LT::swap(_rhs.last_, last_);
			LT::swap(_rhs.node_, node_);
		}
		//******************************************************************************************************************
		//**********************************����������������****************************************************************
		//******************************************************************************************************************
		//-----------------------------------����ȡ�õ�������Ӧ���õĲ�����------------------------------------------------
		reference operator*()const { return *cur_; }
		pointer operator->() const { return *cur_; }

		//-------------------------------------------�����Լ�------------------------------------------------------------------------

		difference_type operator-(const self& _x)const {
			return static_cast<difference_type>(buffer_size() * (node_ - _x.node_ - 1) + cur_ - first_ + _x.last_ - _x.cur_);
		}

		self& operator++() {
			if (++cur_ == last_) {
				set_node(node_ + 1);
				cur_ = first_;
			}
			return *this;
		}
		self& operator++(int) {
			auto it = this;
			if (++cur_ == last_) {
				node_ = set_node(node_ + 1);
				cur_ = first_;
			}
			return *it;
		}
		self& operator--() {
			if (cur_ == first_) {
				set_node(node_ - 1);
				cur_ = last_ - 1;
			}
			--cur_;
			return *this;
		}

		self& operator--(int) {
			auto it = this;
			if (cur_ == first_) {
				node_ = set_node(node_ - 1);
				cur_ = last_ - 1;
			}
			--cur_;
			return *it;
		}

		//--------------------------------------------------ʵ���������------------------------------------------
		self& operator+=(difference_type _n) {
			difference_type offset = _n + (cur_ - first_);
			//�������ж�һ��ƫ���˼���node;
			difference_type nodeOffset = offset / buffer_size();
			if (nodeOffset) {
				set_node(node_ + nodeOffset);
			}
			first_ += (offset % buffer_size());
			return *this;
		}
		//+=����ʽ��+Ҫ�ߣ���Ϊ+��Ҫ���������һ����ʱ����
		self operator+(difference_type _n) {
			auto tmp = *this;
			return tmp += _n;;
		}

		self& operator-=(difference_type _n) {
			return (*this) += -_n;
		}

		self  operator-(difference_type _n) {
			auto tmp = *this;
			return tmp += -_n;;
		}
		
		reference operator[](difference_type _n)const {
			return *(*this + _n);
		}

		//----------------------------------------------------------�Ƚϲ�����---------------------------------------------------
		bool operator==(const self& _rhs)const { return _rhs.cur_ == cur_; }
		bool operator!=(const self& _rhs)const { return _rhs.cur_ != cur_; }
		bool operator<(const self& _rhs)const { return (node_ == _rhs.node_ ? (cur_ < _rhs.cur_) : (node_ < _rhs.node_)); }
		bool operator> (const self& _rhs) const { return _rhs < *this; }
		bool operator<=(const self& _rhs) const { return !(_rhs < *this); }
		bool operator>=(const self& _rhs) const { return !(*this < _rhs); }
	};

	template<typename T, typename Alloc = allocator<T>, size_t _BufSize = 4096>
	class deque {
	public:
		//������������
		typedef T                                    value_type;
		typedef value_type*									 pointer;
		typedef value_type&									 reference;
		typedef const value_type&				     const_reference;
		typedef size_t								 size_type;
		typedef pointer*							 map_pointer;
		typedef ptrdiff_t							 difference_type;
		//���������
		typedef __deque_iterator<value_type, value_type&, value_type*, _BufSize>			    iterator;
		typedef __deque_iterator<value_type, const value_type&, const value_type*, _BufSize>   const_iterator;
		typedef LT::reverse_iterator<iterator>					    reverse_iterator;
		typedef LT::reverse_iterator<const_iterator>			    const_reverse_iterator;
		
	protected:
		//����һЩ�ܿռ���ڲ�������:
		iterator start_;
		iterator finish_;
		map_pointer map_;//ָ��map��ָ�룬map���������ϵ�ռ䡣
		size_type mapSize_;//ָʾmap�����м���node��
	public:
		//************************************************************************************************************
		//*********************************����ӿ�*******************************************************************
		//************************************************************************************************************

		//----------------------------------------���캯��------------------------------------------------------------
	public:
		deque() { __init_n(0); }
		deque(size_type _n){ __init_n(_n); }
		deque(size_type _n, const value_type& _value) { __init_n(_n, _value); }
		template <class InputIter, 
			typename LT::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
		deque(InputIter _first, InputIter _last) { __init_iter(_first, _last); }
		deque(std::initializer_list<value_type> _ilist) { __init_iter(_ilist.begin(), _ilist.end()); }
		deque(const deque& _rhs)
		{
		/*	iterator beg;
			beg.cur_ = (int*)_rhs.begin().cur_;
			beg.first_ = (int*)_rhs.begin().first_;
			beg.last_ = (int*)_rhs.begin().last_;
			iterator end;
			end.cur_ = (int*)_rhs.end().cur_;
			end.first_ = (int*)_rhs.end().first_;
			end.last_ = (int*)_rhs.end().last_;
			__init_iter(beg, end);*/
			__init_iter(_rhs.begin(), _rhs.end());
			
		}
		deque(deque&& _rhs) 
		{
			//������ʱ��ģ����Զ��̳�_bufSize
			start_ = LT::move(_rhs.start_); 
			finish_ = LT::move(_rhs.finish_); 
			map_ = _rhs.map_;
			mapSize_ = _rhs.mapSize_;
			_rhs.map_ = nullptr;
			_rhs.mapSize_ = 0;
		}
		deque& operator=(const deque& _rhs) 
		{
			if (_rhs != *this)
			{
				__init_iter(_rhs.begin(), _rhs.end());
			}
			return *this;
		}
		deque& operator=(deque&& _rhs) 
		{
			start_ == LT::move(_rhs.start_);
			finish_ = LT::move(_rhs.finish_);
			map_ = _rhs.map_;
			mapSize_ = _rhs.mapSize_;
			_rhs.map_ = nullptr;
			_rhs.mapSize_ = 0;
		}
		~deque() { __deallocate_all_mem(map_, map_ + mapSize_); }


		//------------------------------------------------------���������--------------------------------------------
		iterator begin() {	return start_;}
		iterator end() { return finish_; }
		const_iterator begin() const{ return start_; } //��ֹ��ʱ���ǵ���cbegin�����ֱ������
		const_iterator end() const{ return finish_; }
		const_iterator cbegin() const { return begin(); }
		const_iterator cend() const { return end(); }
		reference back() { auto tmp = finish_; return *(--tmp); }
		reference front() { return *start_; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator crbegin() const { return reverse_iterator(end()); }
		const_reverse_iterator crend() const { return reverse_iterator(begin()); }
//
//		//----------------------------------------------------�������-------------------------------------------------
		size_type size() { return finish_ - start_; }
		size_type max_size() { return static_cast<size_type>(-1); }
		bool empty() { return finish_ = start_; }
		void resize(size_type _newSize) { __resize(_newSize, T()); }
		void resize(size_type _newSize, const value_type& _value) { __resize(_newSize, _value); }
		void shrink_to_fit() { __shrink_to_fit(); }
//
//		//----------------------------------------------------Ԫ�ط���------------------------------------------------
//		reference operator[](size_type _n) { return start_[static_cast<difference_type>(_n)]; }
//		reference at(size_type _n) { return (*this)[_n]; }
//
//
//		//----------------------------------------------------Ԫ�ر䶯------------------------------------------------
//		void push_front(const value_type& _value){
//			if (start_.cur_ == start_.first_) {//˵���ռ䲻����,��Ҫ��Ծ����һ��node
//				__create_new_map_front();
//			}
//			LT::construct(LT::address_of(--start_.cur_), _value);
//		}
//		void push_front(const value_type&& _value) { emplace_front(LT::move(_value)); }
//		void push_back(const value_type& _value){
//			if (finish_.cur_ == finish_.last_ - 1) {//˵���ռ䲻����,��Ҫ��Ծ����һ��node
//				__create_new_map_back();
//			}
//			LT::construct(LT::address_of(++finish_.cur_), _value);
//		}
//		void push_back(const value_type&& _value) {
//			emplace_back(LT::move(_value));
//		}
//
//		void pop_back()
//		{
//			if (start_ == finish_) { return; }
//			if (finish_.cur_ == finish_.first_)
//			{
//				map_pointer nodeToDelete = finish_.node_;
//				LT::destroy(LT::address_of(*(--finish_)));
//				data_dealloctor(finish_.node_);//��������Ŀռ�
//			}
//			else {
//				LT::destroy(LT::address_of(*(--finish_)));
//			}
//		}
//		void pop_front()
//		{
//			if (start_ == start_) { return; }
//			if (start_.cur_ == start_.last_ - 1)
//			{
//				map_pointer nodeToDelete = start_.node_;
//				LT::destroy(LT::address_of(*(--start_)));
//				data_dealloctor(start_.node_);//��������Ŀռ�
//			}
//			else {
//				LT::destroy(LT::address_of(*(--start_)));
//			}
//		}
//		template<class ...Args>
//		void emplace_back(Args&&..._args) { __emplace_back(LT::forward<Args>(_args)...); }
//		template<class ...Args>
//		void emplace_front(Args&&..._args) { __emplace_front(LT::forward<Args>(_args)...); }
//		template<class ...Args>
//		iterator emplace(iterator _pos, Args&&..._args) { __emplace(_pos,LT::forward<Args>(_args)...); }
//		iterator insert(iterator _pos, const value_type& _value){ __emplace(_pos, _value); }
//		iterator insert(iterator _pos, const value_type&& _value) { __emplace(_pos,LT::move(_value)); }
//		void insert(iterator _pos, size_type _n, const value_type& _value) { __insert_n(_pos, _n, _value); }
//		template <class InputIterator,
//				 typename std::enable_if<LT::is_input_iterator<InputIterator>::value, int>::type = 0>
//		void insert(iterator _pos, InputIterator _first, InputIterator _last)
//{
//			__insert_by_iter(_pos, _first, _last);
//		}
//		iterator erase(iterator _pos) { __erase(_pos); }
//		iterator erase(iterator _first, iterator _last) { __erase_by_iter(_first, _last); }
//		
//		void assign(size_type _n, const value_type& _value) { __assign_n(_n, _value); }
//		template<class InputIter,
//			typename LT::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
//			void assign(InputIter _first, InputIter _last) { __assign_iter(_first, _last); }
//		void assign(std::initializer_list<value_type> _ilist) { __assign_iter(_ilist.begin(), _ilist.end()); }
//		void swap(deque& _rhs)
//		{
//			if (_rhs != *this)
//			{
//				LT::swap(start_, _rhs.start_);
//				LT::swap(finish_, _rhs.finish_);
//				LT::swap(map_, _rhs.map_);
//				LT::swap(mapSize_, _rhs.mapSize_);
//
//			}
//		}
		void clear() {
			__destroy_by_iter(begin(), end());
			for (map_pointer cur = finish_.node_; cur != start_.node_; --cur)
			{
				data_dealloctor(*cur);
				*cur = nullptr;
			}
			finish_ = start_;
		}

		//�Ƚϲ�����


		//************************************************************************************************************
		//*******************************************************�ڲ�ʵ��**********************************************************
		//************************************************************************************************************

		//��Ҫ�����ռ��������������ռ�������.�������൱�ں�����ģ���Alloc
	protected:
		#define MIN_MAP_NUM 8
		template<class T,class Alloc>//
		inline T* __deque_allocator(size_type _mapSize = iterator::buffer_size()){
			//����ǰ����ٸ�T�������С
			T* tmp =  LT::allocator<T>::allocate(_mapSize);
			assert(tmp);//�ڴ治������Ϊ�ա�
			return tmp;
		}

		//typedef __deque_allocator<value_type, Alloc>	data_alloctor;//����buffer
		//typedef __deque_allocator<pointer, Alloc>		map_alloctor;//����map
		#define data_alloctor  __deque_allocator<value_type, Alloc> //����buffer
		#define map_alloctor   __deque_allocator<pointer, Alloc> //����map

		template<class T,class Alloc>//
		inline void __deque_deallocator(T* _ptr, size_type _size) 
		{
			LT::allocator<T>::deallocate(_ptr,sizeof(T) * _size);
		}

		//typedef __deque_deallocator<value_type, Alloc> data_dealloctor;//����buffer
		//typedef __deque_deallocator<pointer, Alloc> map_dealloctor;//����map
		#define data_dealloctor  __deque_deallocator<value_type, Alloc> //����buffer
		#define map_dealloctor  __deque_deallocator<pointer, Alloc> //����map


		//���������һ���������������ڴ�

		void __construct_by_iter(iterator _itBeg, iterator _itEnd, const value_type _value)
		{
			while (_itBeg != _itEnd) {
				LT::construct(LT::address_of(*_itBeg), _value);
			}
		}
		
		void __destroy_by_iter(iterator _itBeg, iterator _itEnd, const value_type _value)
		{
			LT::destroy(_itBeg, _itEnd);	
		}

		//�ú����ǽ��г�ʼ�����䣬�������ٷ��䣬ָ�������С֮�������������ڸú���֮�󶼻ᱻ�������,mapSizeҲ�ᱻ���ú�
		inline void __create_map_and_nodes(size_type _numElements) {
			size_type nodeNum = _numElements / iterator::buffer_size() + 1;

			//map��Ҫ����Ľڵ���Ӧ��������8�����������������+ 2;
			mapSize_ = max(size_type(MIN_MAP_NUM), nodeNum + 2);//���ó�Ա����_mapSize
			map_ = map_alloctor(mapSize_);
			map_pointer nstart = map_ + (mapSize_ - nodeNum) / 2;
			map_pointer nfinish = nstart + nodeNum - 1;
			map_pointer cur;
			try {
				for (cur = nstart; cur <= nfinish; ++cur) {
					*cur = data_alloctor();
					//*cur = __deque_allocator<value_type, Alloc>();
				}
			}
			catch (...) {
				for (; cur > nstart;) {
					--cur;
					data_dealloctor(*cur,1);//rollback
				}
			}
			//start,cur,finish����ָ���node��㣬��node����ڲ�����Ϣ�ɵ�����ά���������set_node���µ������ڲ�������ָ��
			start_.set_node(nstart);
			finish_.set_node(nfinish);
			start_.cur_ = start_.first_;
			finish_.cur_ = finish_.first_ + _numElements % iterator::buffer_size();
			//ʵ������ͨ��start_.cur_��finish.cur_�������ǰ�˺����˵ĵ�Ԫ�أ���front()��back().�����ı߽�����ͨ��map��ÿ��node�ı�Ե��ȷ��
		}

		//��ǰ�����ں����n���µ�node�ռ�
		inline void __create_new_map_front(size_type _n = 1)
		{
			if (start_.node_ - _n < map_) {//map_ǰ��û�пռ��ˡ�
				size_type distanceBack = finish_.node_ - map_;
				map_pointer newMap = map_alloctor(mapSize_ + _n);
				LT::copy(start_.node_, finish_.node_, newMap + _n);
				map_dealloctor(map_);
				map_ = newMap;
				mapSize_ += _n;
				start_.node_ = (map_ + _n);
				finish_.node_(map_ + distanceBack + _n);
			}
			for (size_type i = 1; i <= _n; ++i)
			{
				if (*(start_.node_ - i) == nullptr)
				{
					*(start_.node_ - i) = data_alloctor();
				}
			}
		}

		inline void __create_new_map_back(size_type _n = 1)
		{
			if (finish_.node_ + _n > map_ + mapSize_) {//map_����û�пռ��ˡ�
				size_type distancePre = start_.node_ - map_;
				size_type distanceMid = finish_.node_ - start_.node_;
				map_pointer newMap = map_alloctor(mapSize_ + _n);
				LT::copy(map_, map_ + mapSize_, newMap + distancePre);
				map_dealloctor(map_);
				map_ = newMap;
				mapSize_ += _n;
				start_.node_ = (map_ + distancePre);
				finish_.node_(map_ + distanceMid + distancePre);
			}
			for (size_type i = 1; i <= _n; ++i)
			{
				if (*(finish_.node_ + i) == nullptr)
				{
					*(finish_.node_ + i) = data_alloctor();
				}
			}
		}
		
		//---------------------------------------------��һ��������г�ʼ��,ֵ���---------------------------------------
		inline void __construct(iterator _first, iterator _last, value_type _value)
		{
			//��Ȼ�ú����ṩ�Ľӿڿ������ǿ��Զ�һ�������������������г�ʼ��������ʵ�����ڲ�����Ҫ��һС��һС�����ɢ�ռ���г�ʼ��
			if (_first.node_ == _last.node_) {
				LT::uninitialized_fill(_first.cur_, _last.cur_, _value);
			}
			else {
				LT::uninitialized_fill(_first.cur_, _first.last_, _value);//ͷ��
				for (map_pointer it = _first.node_; it < _last.node_; ++it) {
					LT::uninitialized_fill(*it, *it + iterator::buffer_size(), _value);//�м�����
				}
				LT::uninitialized_fill(_last.cur_, _last.last_, _value);//β��
			}
		}

		//ע�⣬��������ǰ���ǣ�_last - _first == _InputEnd -  _Inputfirst�����ǵ����ܡ�û�ж��μ�顣
		template <class InputIterator>
		inline void __construct(InputIterator _inputFirst, InputIterator _inputEnd, iterator _first, iterator _last)
		{
			auto it = _first;
			try {
				for (; it > _last; ++it, ++_inputFirst) {
					LT::construct(LT::address_of(*it), *_inputFirst);
				}
			}
			catch (...) {
				for (; it > _first; --it) {
					LT::destroy(LT::address_of(*it));
				}
			}
		}

		template <>
		inline void __construct(const_iterator _inputFirst, const_iterator _inputEnd, iterator _first, iterator _last) 
		{
			auto it = _first;
			try {
				for (; it > _last; ++it, ++_inputFirst) {
					LT::construct(LT::address_of(*it), *_inputFirst);
				}
			}
			catch (...) {
				for (; it > _first; --it) {
					LT::destroy(LT::address_of(*it));
				}
			}
		}

		//------------------------------------------- ��ʼ��----------------------------------------------------------------
		inline void __init_n(size_type _numElements, value_type _value = T()) 
		{
			__create_map_and_nodes(_numElements);
			__construct(start_, finish_, _value);
		}

		template<typename InputIterator>
		inline void __init_iter(InputIterator _first, InputIterator _last)
		{
			size_type numElements = static_cast<size_type>(_last - _first);
			__create_map_and_nodes(numElements);
			__construct(_first, _last, start_, finish_);
		}
		template<>
		inline void __init_iter(const_iterator _first, const_iterator _last)
		{
			size_type numElements = static_cast<size_type>(_last - _first);
			__create_map_and_nodes(numElements);
			__construct(_first, _last, start_, finish_);
		}
		//---------------------------------------������ɾ��-----------------------------------------------------------------
		//�����ǽ�һ���ڴ������е�����Ԫ����������
		inline void __destroy_mem(iterator _first, iterator _last) {
			if (_first.node_ == _last.node_) {
				LT::destroy(_first.cur_, _last.cur_);
			}
			else {
				LT::destroy(_first.cur_, _first.last_);//ͷ��
				for (map_pointer it = _first.node_; it < _last.node_; ++it) {
					LT::destroy(*it, *it + __deque_iterator::buffer_size());//�м�����
				}
				LT::destroy(_last.cur_, _first.last_);//β��
			}
		}

		//���ǻ���һ��һ����ڴ�ռ䡣���յ���С��λ��node�����������map��ֻ����ĳ��map�ڵ��Ӧ��node
		inline void __deallocate_mem(map_pointer _first, map_pointer _last) {
			for (; _first != _last; ++_first) {
				data_dealloctor(_first);
				_first = nullptr;
			}
		}

		inline void __deallocate_all_mem(map_pointer _first, map_pointer _last) {
			for (; _first != _last; ++_first) {
				data_dealloctor(*_first, 1);
			}
			map_dealloctor(_first,mapSize_);//��һ�����ÿһ���ڵ�������ڴ�ռ���������
			map_ = nullptr;
			start_.clear();
			finish_.clear();
			mapSize_ = 0;
		}

		//---------------------------------------------------------------�ڴ��ƶ�--------------------------------------------------------------------------
		
		//��_pos��ʼ�����潫������_n���հ��ڴ档����������ʧЧ�������Ҫ����һ��������
		//�ú�����_pos����ȡbegin��end
		iterator __get_n_mem(iterator _pos, size_type _n)
		{
			
			size_type preSize = _pos - start_;
			size_type backSize = finish_ - _pos;

			if (preSize < backSize)
			{
				//ȷ��ǰ����n�����пռ�
				size_type curMapRes = start_.cur - start_.first_;
				if (_n > curMapRes)
				{
					size_type newMapNum = (_n - curMapRes) / __deque_iterator::buffer_size();
					if (newMapNum)
					{
						__create_new_map_front(newMapNum);
					}
				}
				//�ռ䷽���Ѿ�׼������ˣ�����Ҫ��ʼǨ��
				_pos = start_ + preSize;
				LT::uninitialized_move(start_, start_ + preSize, start_ - min(_n, preSize));
				if (preSize > _n)
				{
					LT::move(start_ + _n, _pos, start_);
					__destroy_by_iter(_pos - _n + 1, _pos + 1);
				}
				
			}
			else
			{
				//ȷ�Ϻ�����n�����пռ�
				size_type curMapRes = finish_.last_ - finish_.cur_;
				if (_n > curMapRes)
				{
					size_type newMapNum = (_n - curMapRes) / __deque_iterator::buffer_size();
					if (newMapNum)
					{
						__create_new_map_back(newMapNum);
					}
				}
				_pos = start_ + preSize;
				LT::uninitialized_move(finish_ - backSize, finish_, finish_ + min(_n, backSize));
				if (backSize > _n)
				{
					LT::move_backward(_pos, finish_ - _n, finish_);
					__destroy_by_iter(_pos, _pos + _n);
				}
				_pos -= (_n - 1);
			}

			return _pos;
		}

		//---------------------------------------------------------------�ӿ�ʵ��--------------------------------------------------------------------------
		
		//resize
	//	inline void __resize(size_type _newSize, const value_type& _value)
	//	{
	//		size_type oldSize = size();
	//		if (_newSize > oldSize)
	//		{
	//			erase(start_ + _newSize, finish_);
	//		}
	//		else  if(oldSize < _newSize)
	//		{
	//			insert(finish_, _newSize - oldSize, _value);
	//		}
	//	}

		//shrink_to_fit
		inline void __shrink_to_fit()
		{
			map_pointer startMap = start_.node_;
			map_pointer endMap = finish_.node_;

			for (map_pointer cur = map_; cur < startMap; ++cur)
			{
				map_dealloctor(*cur, __deque_iterator::buffer_size());
				*cur = nullptr;
			}

			for (map_pointer cur = endMap + 1; cur < map_ + mapSize_ ; ++cur)
			{
				if (*cur)
				{
					map_dealloctor(*cur, __deque_iterator::buffer_size());
					*cur = nullptr;
				}
				else {
					break;
				}
			}
		}

	//	//emplace���
	//	template<class ...Args>
	//	inline void __emplace_front(Args&& ..._args)
	//	{
	//		if (start_.cur_ == start_.first_) {//˵���ռ䲻����,��Ҫ��Ծ����һ��node
	//			__create_new_map_front();
	//		}
	//		LT::construct(LT::address_of(--start_.cur_), LT::forward<Args>(_args)...);
	//	}

	//	template<class ...Args>
	//	inline void __emplace_back(Args&& ..._args)
	//	{
	//		if (finish_.cur_ == finish_.first_) {//˵���ռ䲻����,��Ҫ��Ծ����һ��node
	//			__create_new_map_back();
	//		}
	//		LT::construct(LT::address_of(++finish_.cur_), LT::forward<Args>(_args)...);
	//	}

	//	template<class ...Args>
	//	inline iterator __emplace(iterator _pos, Args&&..._args)
	//	{
	//		if (_pos == cend())
	//		{
	//			emplace_back(LT::forward<Args>(_args)...);
	//			return begin();
	//		}
	//		else if (_pos == cbegin()) {
	//			emplace_front(LT::forward<Args>(_args)...);
	//			return end();
	//		}
	//		iterator pos = __get_n_mem(_pos, 1);
	//		LT:construct(LT::address_of(*pos), LT::forward<Args>(_args)...);
	//		return pos;
	//	}

	//	inline void __insert_n(iterator _pos, size_type _n, const value_type& _value)
	//	{
	//		if (_pos == begin())
	//		{
	//			while (_n)
	//			{
	//				push_front(_value);
	//				--_n;
	//			}
	//		}else if (_pos == end())
	//		{
	//			while (_n)
	//			{
	//				push_back(_value);
	//				--_n;
	//			}
	//		}
	//		else
	//		{
	//			iterator pos = __get_n_mem(_n);
	//			LT::uninitialized_fill_n(pos, _n, _value);
	//		}
	//	}

	//	template<class InputIter>
	//	inline void __insert_by_iter(iterator _pos, InputIter _itBeg, InputIter _itEnd)
	//	{
	//		if (_itEnd <= _itBeg) { return; }
	//		if (_pos == begin())
	//		{
	//			for (; _itBeg != _itEnd; ++_itBeg)
	//			{
	//				push_front(*_itBeg);
	//			}
	//		}
	//		else if (_pos == end())
	//		{
	//			for (; _itBeg != _itEnd; ++_itBeg)
	//			{
	//				push_back(*_itBeg);
	//			}
	//		}
	//		else
	//		{
	//			size_type n = static_cast<size_type>(LT::distance(_itBeg, _itEnd));
	//			iterator pos = __get_n_mem(n);
	//			LT::uninitialized_copy(_itBeg, _itEnd, pos);
	//		}
	//	}

	//

	//	inline iterator __erase(iterator _pos)
	//	{
	//		size_type preSize = _pos - begin();
	//		size_type backSize = _pos - end();
	//		if (preSize < backSize)
	//		{
	//			LT::move(begin(), _pos, begin() - 1);
	//			pop_front();
	//		}
	//		else {
	//			LT::move_backward(_pos + 1, end(), _pos);
	//			pop_back();
	//		}
	//	}
	//	
		inline iterator __erase(iterator _itBeg, iterator _itEnd)
		{
			if (_itBeg == start_ && _itEnd == finish_)
			{
				clear();
				return finish_;
			}
			else
			{
				difference_type n = _itEnd - _itBeg;
				difference_type preSize = _itBeg - start_;
				difference_type backSize = finish_ - _itEnd;
				if (preSize < backSize)
				{
					iterator newStart = start_ + n;
					LT::move_backward(start_, _itBeg, _itEnd);
					destroy(start_, newStart);
					for (map_pointer cur = start_.node_; cur != newStart.node_; ++cur)
					{
						data_dealloctor(*cur);
						*cur = nullptr;
					}
					start_ = newStart;
				}
				else {
					iterator newEnd = finish_ - n;
					LT::move(_itEnd, finish_, _itBeg);
					destroy(newEnd, finish_);
					for (map_pointer cur = finish_.node_; cur != newEnd.node_; --cur)
					{
						data_dealloctor(*cur);
						*cur = nullptr;
					}
					finish_ = newEnd;
				}

				return _itBeg;
			}
		}

	//assign
		inline void __assign_n(size_type _n, const value_type& _value)
		{
			if (_n > size())
			{
				LT::fill(start_, finish_, _value);
				insert(end(), _n - size(), _value);
			}
			else {
				LT::fill(start_, start_ + _n, _value);
				erase(begin() + _n, end());
			}
		}

		template<class InputIter>
		inline void __assign_iter_dispatch(InputIter _itBeg, InputIter _itEnd, input_iterator_tag)
		{
			iterator cur = start_;
			iterator end = finish_;
			for (; cur != end && _itBeg != _itEnd; ++cur, ++_itBeg)
			{
				*cur = *_itBeg;
			}
			if (cur != end)
			{
				erase(cur, end);
			}
			else {
				__insert_by_iter(finish_, _itBeg, _itEnd);
			}
		}

		template <class ForwardIter>
		inline void __assign_iter_dispatch(ForwardIter _itBeg, ForwardIter _itEnd, forward_iterator_tag)
		{
			size_type size = size();
			size_type InputSize = static_cast<size_type>(LT::distance(_itBeg, _itEnd));
			if (size < InputSize)
			{
				ForwardIter mid = _itBeg;
				LT::advance(mid, size);
				LT::copy(_itBeg, mid, start_);
				__insert_by_iter(finish_, mid, _itEnd);
			}
			else {
				__erase(LT::copy(_itBeg, _itEnd, start_), finish_);
			}
		}
	};


	//------------------------------------------------------�ⲿ����-------------------------------------------------------
	//swap
	//template<class T, class Alloc, size_t _BufSize >
	//void swap(deque<T, Alloc, _BufSize>& _lhs, deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	_lhs.swap(_rhs);
	//}

	////�Ƚϲ�����
	//template<class T, class Alloc, size_t _BufSize >
	//bool operator==(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return _lhs.size() == _rhs.size() && LT::equal(_lhs.cbegin(), _lhs.cend(), _rhs.cbegin());
	//}

	//template<class T, class Alloc, size_t _BufSize >
	//bool operator<(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return LT::lexicographical_compare(_lhs.begin(), _lhs.end(), _rhs.begin(), _rhs.end());
	//}

	//template<class T, class Alloc, size_t _BufSize >
	//bool operator!=(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return !(_lhs == _rhs);
	//}

	//template<class T, class Alloc, size_t _BufSize >
	//bool operator>(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return _rhs < _lhs;
	//}

	//template<class T, class Alloc, size_t _BufSize >
	//bool operator<=(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return !(_rhs < _lhs);
	//}

	//template<class T, class Alloc, size_t _BufSize >
	//bool operator>=(const deque<T, Alloc, _BufSize>& _lhs, const deque<T, Alloc, _BufSize>& _rhs)
	//{
	//	return !(_lhs < _rhs);
	//}
}
