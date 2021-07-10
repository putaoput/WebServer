//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件负责动态空间配置

//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
#include<atomic>
#include "construct.h"
#include "allocator.h"
//该头文件负责提供三种智能指针的实现，shared_ptr, weak_ptr，unique_ptr，(auto_ptr已经被抛弃)
//提供一些内存处理函数：address_of


namespace LT {
	//提供address_of
	template<class T>
	T* address_of(T& _value) {
		return &_value;
	}

	class DefaultDeleter {
	public:
		template<class T>
		void operator()(T* _ptr)
		{
			delete _ptr;
		}
	};

	class DestructorDeleter {
	public:
		template<class T>
		void operator()(T* _ptr)
		{
			_ptr->~T();
		}
	};
	//前置声明
	template<class T, class Deleter = DefaultDeleter()>
	class weak_ptr;

	//------------------------------------------------shared_ptr-------------------------------------------------------
	template<class T, class Deleter = DefaultDeleter>
	class shared_ptr_impl {
	public:
		//定义一些类型
		typedef T					value_type;
		typedef	value_type*         pointer;
		typedef value_type&			reference;
		typedef size_t				size_type;
	public:
		constexpr shared_ptr_impl() = default;

		shared_ptr_impl(pointer _ptr , Deleter _del = Deleter())
			:ptr_(_ptr),pCount_(1),deleter_(_del){}
		template<class ...Args>
		shared_ptr_impl(pointer _ptr , Deleter _del = Deleter(), Args&&... _args)
			:ptr_(_ptr), pCount_(1), deleter_(_del) 
		{
			construct(_ptr, LT::forward<Args>(_args)...);
		}

		~shared_ptr_impl()
		{
			deleter_()(ptr_);
		}

		//操作符重载
		pointer operator->()const
		{
			return ptr_;
		}

		reference operator*()const
		{
			return *ptr_;
		}

		//对外接口
		pointer get()const
		{
			return ptr_;
		}

		void hold()
		{
			++pCount_;
		}

		bool release()
		{
			--pCount_;
			return pCount_;
		}

	private:
		pointer ptr_;
		std::atomic<size_type> pCount_;
		//std::function<void(pointer)> deleter_;
		Deleter deleter_;

	};
	
	template<class T, class Deleter = DefaultDeleter>
	class shared_ptr {
		//友元
		friend class  weak_ptr<T, DefaultDeleter>;
	public:
		//定义一些类型
		typedef typename shared_ptr_impl<T,Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;

		//构造函数
		constexpr shared_ptr() = default;

		explicit shared_ptr(shared_ptr_impl<T,Deleter>* _sPtr):sPtr_(_sPtr){}
		
		explicit shared_ptr(pointer _ptr = nullptr, Deleter _del = Deleter()):sPtr_(new shared_ptr_impl(_ptr, _del)){}
		
		template <class U>//为了限定只能传递指针
		explicit shared_ptr(U* _ptr = nullptr, Deleter _del = Deleter()) :sPtr_(new shared_ptr_impl(_ptr, _del)) {}

		shared_ptr(const shared_ptr& _rhs)
			:sPtr_(_rhs.sPtr_)
		{
			_rhs.sPtr_.hold();
		}

		~shared_ptr()
		{
			if (sPtr_.release())
			{
				delete sPtr_;
			}
		}

		template<class U>
		shared_ptr(const shared_ptr<U, Deleter> _rhs)
			:sPtr_(_rhs.sPtr_)
		{
			_rhs.sPtr_.hold();
		}
		
		shared_ptr& operator=(const shared_ptr& _rhs)
		{
			sPtr_ = _rhs.sPtr_;
			_rhs.sPtr_.hold();
			return *this;
		}

		template<class U>
		shared_ptr<U,Deleter>& operator=(const shared_ptr<U,Deleter>& _rhs)
		{
			sPtr_ = _rhs.sPtr_;
			_rhs.sPtr_.hold();
			return *this;
		}

		//---------------------------------------------------对外接口-----------------------------------------------------------
		//操作符重载
		pointer operator->()const
		{
			return this->sPtr_;
		}
		reference operator*()const
		{
			return **sPtr_;
		}

		template<class U>
		bool operator<(const shared_ptr<U, Deleter>& _rhs)
		{
			return sPtr_->get() < _rhs.sPtr_->get();
		}

		pointer get()const
		{
			return sPtr_->get();
		}

		void reset(pointer _ptr, Deleter = Deleter())
		{
			shared_ptr tmp;
			swap(tmp);
		}

		void swap(shared_ptr& _rhs)
		{
			LT::swap(_rhs.sPtr_, sPtr_);
		}

		size_type use_count()const
		{
			return sPtr_ ? sPtr_->pCount_ : 0;
		}

		bool unique()
		{
			return this->use_count() == 1;
		}
	private:
		//成员变量
		shared_ptr_impl<T,Deleter>* sPtr_; //智能指针的核心和关键就在于按位拷贝了一个指针，即浅拷贝的指针。每次拷贝该指针改变引用计数，因为++i， --i是线程安全的，所以可以在计数为零是析构。
	};


	//------------------------------------------------------weak_ptr-----------------------------------------------------------------------------------------------------------------------
	template<class T, class Deleter = DefaultDeleter()>
	class weak_ptr
	{
	public:
	//定义一些类型
		typedef typename shared_ptr_impl<T, Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;
		
	//构造函数
		constexpr weak_ptr() = default;

		weak_ptr(const weak_ptr& _rhs)
			:sPtr_(_rhs.sPtr_){}

		weak_ptr(const shared_ptr<T,Deleter>& _rhs)
			:sPtr_(_rhs.sPtr_) {}

		weak_ptr& operator=(const weak_ptr& _rhs)
		{
			sPtr_ = _rhs.sPtr_;
			return *this;
		}

		~weak_ptr() = default;

		

	//对外接口
		size_type use_count()const
		{
			return sPtr_->pCount_;
		}

		bool expired()const
		{
			return use_count() == 0;
		}

		shared_ptr<T, Deleter> lock()
		{
			return shared_ptr<T, Deleter>(static_cast<shared_ptr<T,Deleter>>(sPtr_));
		}

		void swap(const weak_ptr& _rhs)
		{
			LT::swap(_rhs.sPtr_, sPtr_);
		}
		
		void reset()
		{
			swap(weak_ptr());
		}
	private:
		shared_ptr_impl<T, Deleter>* sPtr_;
	};

	//---------------------------------------------------------------unique_ptr------------------------------------------------
	template<class T, class Deleter = DefaultDeleter>
	class unique_ptr {
	public:
		//定义一些类型
		typedef typename shared_ptr_impl<T, Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;

		//构造函数
		constexpr unique_ptr() = default;
		
		explicit unique_ptr(pointer _ptr , Deleter _del = Deleter()) :sPtr_(new shared_ptr_impl(_ptr, _del)) {}

		template <class U>//为了限定只能传递指针
		explicit unique_ptr(U* _ptr , Deleter _del = Deleter()) :sPtr_(new shared_ptr_impl(_ptr, _del)) {}


		~unique_ptr()
		{
			if (sPtr_.release())
			{
				delete sPtr_;
			}
		}

		//大部分没有区别，都是不能在外部访问这个函数。
		//区别在于：只在使用模板函数时，只能采用delete特化，不能采用private特化。
		
		unique_ptr(const unique_ptr& _rhs) = delete;

		unique_ptr& operator=(const unique_ptr& _rhs) = delete;
		

		//---------------------------------------------------对外接口-----------------------------------------------------------
		//操作符重载
		pointer operator->()const
		{
			return this->sPtr_;
		}
		reference operator*()const
		{
			return **sPtr_;
		}

		template<class U>
		bool operator<(const unique_ptr<U, Deleter>& _rhs)
		{
			return sPtr_->get() < _rhs.sPtr_->get();
		}

		pointer get()const
		{
			return sPtr_->get();
		}

		void reset(pointer _ptr, Deleter = Deleter())
		{
			unique_ptr tmp;
			swap(tmp);
		}

		void swap(unique_ptr& _rhs)
		{
			LT::swap(_rhs.sPtr_, sPtr_);
		}

		pointer release()
		{
			pointer ptr = pointer();
			LT::swap(ptr, sPtr_->ptr_);
			return ptr;
		}
		Deleter get_deleter()const
		{
			return Deleter();
		}
	private:
		//成员变量
		shared_ptr_impl<T, Deleter>* sPtr_; //智能指针的核心和关键就在于按位拷贝了一个指针，即浅拷贝的指针。每次拷贝该指针改变引用计数，因为++i， --i是线程安全的，所以可以在计数为零是析构。
	};


	//-------------------------------------------------------外部重载-------------------------------------------------------

	template <class T, class Deleter>
	void swap(weak_ptr<T, Deleter> _lhs, weak_ptr<T, Deleter> _rhs)
	{
		_lhs.swap(_rhs);
	}

	template <class T, class Deleter>
	void swap(shared_ptr<T, Deleter> _lhs, shared_ptr<T, Deleter> _rhs)
	{
		_lhs.swap(_rhs);
	}

	
	template <class T, class Deleter>
	void swap(unique_ptr<T, Deleter> _lhs, unique_ptr<T, Deleter> _rhs)
	{
		_lhs.swap(_rhs);
	}
	
	//make_shared
	template<class T, class ... Args>
	shared_ptr<T> make_shared(Args&&... _args)
	{
		
		T* ptr = allocator<T>::allocate();
		assert(ptr == nullptr);
		try {
			LT::construct(ptr, LT::forward<Args>(_args)...);
			return shared_ptr<T>(ptr);
		}
		catch (...)
		{
			LT::destroy(ptr, sizeof(*ptr));
		}
		
	}
}







