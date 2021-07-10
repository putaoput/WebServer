//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件提供配接器stack
#include "deque.h"
namespace LT {
	template<class T, class Sequence = deque<T>>
	class stack {
	public:
		//照例定义一些基本类型
		typedef typename Sequence::value_type						  value_type;
		typedef typename Sequence::reference						  reference;
		typedef typename Sequence::size_type						  size_type;
		typedef typename Sequence::const_reference                    const_reference;

		typedef stack<T>											  self;
	protected:
		Sequence container_;
	public:
		//------------------------------------------构造析构类函数------------------------------------
		stack() = default;
		explicit stack(size_type _n) :container_(_n) {}
		stack(size_type _n, const value_type& _value) :container_(_n, _value) {};
		template<class InputIterator>
		stack(InputIterator _first, InputIterator _last) :container_(_first, _last) {}
		stack(std::initializer_list<T> _ilist) :container_(_ilist.begin(), _ilist.end()) {}
		stack(const stack& _rhs) :container_(_rhs.container_) {}
		stack(stack&& _rhs) :container_(LT::move(_rhs.container_)) {}
		stack(const Sequence& _rhs):container_(_rhs){}
		stack(Sequence&& _rhs) :container_(LT::move(_rhs)) {}
		stack operator=(const stack& _rhs) { container_ = _rhs.container_; return *this; }
		stack operator=(stack&& _rhs) { container_ = LT::move(_rhs.container_); }
		~stack() = default;

		//-------------------------------------------对外接口-------------------------------------------

		reference top() { return container_.back(); }
		const_reference top() const { return container_.back(); }

		bool      empty() const { return container_.empty(); }
		size_type size()  const { return container_.size(); }
		void clear() { container_.clear(); }
		//元素修改
		void push(const value_type& _value) { container_.push_back(_value); }
		void push(value_type&& value) { container_.emplace_back(LT::move(value)); }
		template<class ...Args>
		void emplace(Args&& ..._args)
		{
			container_.emplace_back(LT::forward<Args>(_args)...);
		}
		void pop() { container_.pop_back(); }

		//swap
		void swap(stack& _rhs)
		{
			container_.swap(_rhs.container_);
		}

		//----------------------------------------------------重载比较操作符---------------------------------------------------------
		bool operator==(const stack& _rhs) { return _rhs.container_ == container_; }
		bool operator!=(const stack& _rhs) { return _rhs.container_ != container_; }
		bool operator>(const stack& _rhs) { return _rhs.container_ > container_; }
		bool operator>=(const stack& _rhs) { return _rhs.container_ >= container_; }
		bool operator<(const stack& _rhs) { return _rhs.container_ < container_; }
		bool operator<=(const stack& _rhs) { return _rhs.container_ <= container_; }

	};

	//------------------------------------------------外部重载-----------------------------------------------------------------------
	template<class T, class Container>
	void swap(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		_lhs.swap(_rhs);
	}

	//操作符重载
	template<class T, class Container>
	void operator==(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator==(_rhs);
	}
	template<class T, class Container>
	void operator!=(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator!=(_rhs);
	}
	template<class T, class Container>
	void operator<(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator<(_rhs);
	}
	template<class T, class Container>
	void operator<=(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator<=(_rhs);
	}
	template<class T, class Container>
	void operator>(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator>(_rhs);;
	}
	template<class T, class Container>
	void operator>=(stack<T, Container>& _lhs, stack<T, Container>& _rhs)
	{
		return _lhs.operator>=(_rhs);
	}
	
}
