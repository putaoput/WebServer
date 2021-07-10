//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件提供priority_queue和priortiy_priority_queue两个配接器
#include "deque.h"
#include "vector.h"
#include "functional.h"

namespace LT {
	template<class T, class Sequence = deque<T>>
	class queue {
	public:
		//照例定义一些基本类型
		typedef typename Sequence::value_type						  value_type;
		typedef typename Sequence::reference						  reference;
		typedef typename Sequence::size_type						  size_type;
		typedef typename Sequence::const_reference                    const_reference;

		typedef queue<T>											  self;
	protected:
		Sequence container_;
	public:
		//------------------------------------------构造析构类函数------------------------------------
		queue() = default;
		explicit queue(size_type _n) :container_(_n) {}
		queue(size_type _n, value_type& _value) :container_(_n, _value) {};
		template<class InputIterator>
		queue(InputIterator _first, InputIterator _last) :container_(_first, _last) {}
		queue(std::initializer_list<T> _ilist) :container_(_ilist.begin(), _ilist.end()) {}
		queue(const queue& _rhs) :container_(_rhs.container_) {}
		queue(queue&& _rhs) :container_(LT::move(_rhs.container_)) {}
		queue(const Sequence& _rhs):container_(_rhs){}
		queue(Sequence&& _rhs) :container_(LT::move(_rhs)) {}
		queue operator=(const queue& _rhs) { container_ = _rhs.container_; return *this; }
		queue operator=(queue&& _rhs) { container_ = LT::move(_rhs.container_); }
		~queue() = default;

		//-------------------------------------------对外接口-------------------------------------------

		reference front() { return container_.front(); }
		const_reference front() const { return container_.front(); }
		reference back() { return container_.back(); }
		const_reference back() const { return container_.back(); }
		
		bool      empty() const{ return container_.empty(); }
		size_type size()  const{ return container_.size(); }
		void clear() { container_.clear(); }
		//元素修改
		void push(const value_type& _value) { container_.push_back(_value); }
		void push(value_type&& value) { container_.emplace_back(LT::move(value)); }
		template<class ...Args>
		void emplace(Args&& ..._args){container_.empalce_back(LT::forward<Args>(_args)...}
		void pop() { container_.pop_front(); }

		void swap(queue& _rhs) { container_.swap(_rhs.container_); }

		//----------------------------------------------------重载比较操作符---------------------------------------------------------
		bool operator==(const queue& _rhs) { return _rhs.container_ == container_; }
		bool operator!=(const queue& _rhs) { return _rhs.container_ != container_; }
		bool operator>(const queue& _rhs) { return _rhs.container_ > container_; }
		bool operator>=(const queue& _rhs) { return _rhs.container_ >= container_; }
		bool operator<(const queue& _rhs) { return _rhs.container_ < container_; }
		bool operator<=(const queue& _rhs) { return _rhs.container_ <= container_; }

	};
	//外部重载
	template<class T, class Container>
	void swap(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		_lhs.swap(_rhs);
	}

	//操作符重载
	template<class T, class Container>
	void operator==(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator==(_rhs);
	}
	template<class T, class Container>
	void operator!=(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator!=(_rhs);
	}
	template<class T, class Container>
	void operator<(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator<(_rhs);
	}
	template<class T, class Container>
	void operator<=(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator<=(_rhs);
	}
	template<class T, class Container>
	void operator>(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator>(_rhs);;
	}
	template<class T, class Container>
	void operator>=(queue<T, Container>& _lhs, queue<T, Container>& _rhs)
	{
		return _lhs.operator>=(_rhs);
	}
	//*****************************************************************************************************************************************************
	//---------------------------------------------------------------------priority_queue-----------------------------------------------------------------
	//*****************************************************************************************************************************************************
	template<class T, class Sequence = vector<T>, class Comp = LT::less<typename Sequence::value_type>>
	class priority_queue {
	public:
		//照例定义一些基本类型
		typedef typename Sequence::value_type						  value_type;
		typedef typename Sequence::reference						  reference;
		typedef typename Sequence::size_type						  size_type;
		typedef typename Sequence::const_reference                    const_reference;

	protected:
		Sequence container_;
		Comp cmp_;//注意永远是cmp_(child, parent) = true;
	public:
		//------------------------------------------构造析构类函数------------------------------------
		priority_queue() = default;
		explicit priority_queue(size_type _n) 
			:container_(_n) { LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(size_type _n, value_type& _value) 
			:container_(_n, _value) { LT::__make_heap(container_.begin(), container_.end(), cmp_); };
		template<class InputIterator>
		priority_queue(InputIterator _first, InputIterator _last)
			:container_(_first, _last){ LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(std::initializer_list<T> _ilist)
			:container_(_ilist.begin(),_ilist.end()){ LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(priority_queue& _rhs)
			:container_(_rhs.container_){ LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(const priority_queue& _rhs)
			:container_(_rhs.container_) { LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(priority_queue&& _rhs)
			:container_(LT::move(_rhs.container_)){ LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(const Sequence& _rhs)
			:constainer(_rhs){} { LT::__make_heap(container_.begin(), container_.end(), cmp_); }
		priority_queue(priority_queue&& _rhs)
			:container_(LT::move(_rhs)) {
			LT::__make_heap(container_.begin(), container_.end(), cmp_);
		}
		priority_queue operator=(const priority_queue& _rhs)
		{ container_ = _rhs.container_; LT::__make_heap(container_.begin(), container_.end(), cmp_); return *this; }
		priority_queue operator=(priority_queue&& _rhs)
		{
			container_ = LT::move(_rhs.container_); LT::__make_heap(container_.begin(), container_.end(), cmp_); return *this;
		}
		~priority_queue() = default;

		//-------------------------------------------对外接口-------------------------------------------

		reference top() { return container_.front(); }
		const_reference top() const { return container_.front(); }
		bool empty() const { return container_.empty(); }
		size_type size()  const { return container_.size(); }
		void clear() { container_.clear(); }
		//元素修改
		void push(const value_type& _value) 
		{
			container_.push_back(_value); 
			LT::push_heap(container_.begin(), container_.end(), cmp_); 
		}
		void push(value_type&& value){
			container_.push_back(LT::move(value));
			LT::push_heap(container_.begin(), container_.end(), cmp_);
		}
		void pop() 
		{ 
			LT::iter_swap(container_.front(), container_.back());
			container_.pop_front(); 
			LT::pop_heap(container_.begin(), container_.end(), cmp_);
		}

		template<class ...Args>
		void emplace(Args&& ..._args)
		{
			container_.emplace_back(forward<Args>(_args)...);
			LT::push_heap(container_.begin(), container_.end(), cmp_);
		}
		void swap(priority_queue& _rhs)
		{
			container_.swap(_rhs.container_);
		}
		//----------------------------------------------------重载比较操作符---------------------------------------------------------
		bool operator==(const priority_queue& _rhs) { return _rhs.container_ == container_; }
		bool operator!=(const priority_queue& _rhs) { return _rhs.container_ != container_; }
		bool operator>(const priority_queue& _rhs) { return _rhs.container_ > container_; }
		bool operator>=(const priority_queue& _rhs) { return _rhs.container_ >= container_; }
		bool operator<(const priority_queue& _rhs) { return _rhs.container_ < container_; }
		bool operator<=(const priority_queue& _rhs) { return _rhs.container_ <= container_; }
		//----------------------------------------------内部实现----------------------------------------------------------------------
	private:
		//堆相关函数定义在stl_heap
	};

	//外部重载
	template<class T, class Container>
	void swap(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		_lhs.swap(_rhs);
	}

	//操作符重载
	template<class T, class Container>
	void operator==(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator==(_rhs);
	}
	template<class T, class Container>
	void operator!=(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator!=(_rhs);
	}
	template<class T, class Container>
	void operator<(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator<(_rhs);
	}
	template<class T, class Container>
	void operator<=(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator<=(_rhs);
	}
	template<class T, class Container>
	void operator>(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator>(_rhs);;
	}
	template<class T, class Container>
	void operator>=(priority_queue<T, Container>& _lhs, priority_queue<T, Container>& _rhs)
	{
		return _lhs.operator>=(_rhs);
	}
}