//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ�����̬�ռ�����

//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
#include<atomic>
#include "construct.h"
#include "allocator.h"
//��ͷ�ļ������ṩ��������ָ���ʵ�֣�shared_ptr, weak_ptr��unique_ptr��(auto_ptr�Ѿ�������)
//�ṩһЩ�ڴ洦������address_of


namespace LT {
	//�ṩaddress_of
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
	//ǰ������
	template<class T, class Deleter = DefaultDeleter()>
	class weak_ptr;

	//------------------------------------------------shared_ptr-------------------------------------------------------
	template<class T, class Deleter = DefaultDeleter>
	class shared_ptr_impl {
	public:
		//����һЩ����
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

		//����������
		pointer operator->()const
		{
			return ptr_;
		}

		reference operator*()const
		{
			return *ptr_;
		}

		//����ӿ�
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
		//��Ԫ
		friend class  weak_ptr<T, DefaultDeleter>;
	public:
		//����һЩ����
		typedef typename shared_ptr_impl<T,Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;

		//���캯��
		constexpr shared_ptr() = default;

		explicit shared_ptr(shared_ptr_impl<T,Deleter>* _sPtr):sPtr_(_sPtr){}
		
		explicit shared_ptr(pointer _ptr = nullptr, Deleter _del = Deleter()):sPtr_(new shared_ptr_impl(_ptr, _del)){}
		
		template <class U>//Ϊ���޶�ֻ�ܴ���ָ��
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

		//---------------------------------------------------����ӿ�-----------------------------------------------------------
		//����������
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
		//��Ա����
		shared_ptr_impl<T,Deleter>* sPtr_; //����ָ��ĺ��ĺ͹ؼ������ڰ�λ������һ��ָ�룬��ǳ������ָ�롣ÿ�ο�����ָ��ı����ü�������Ϊ++i�� --i���̰߳�ȫ�ģ����Կ����ڼ���Ϊ����������
	};


	//------------------------------------------------------weak_ptr-----------------------------------------------------------------------------------------------------------------------
	template<class T, class Deleter = DefaultDeleter()>
	class weak_ptr
	{
	public:
	//����һЩ����
		typedef typename shared_ptr_impl<T, Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;
		
	//���캯��
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

		

	//����ӿ�
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
		//����һЩ����
		typedef typename shared_ptr_impl<T, Deleter>::value_type					value_type;
		typedef	typename shared_ptr_impl<T, Deleter>::pointer					pointer;
		typedef typename shared_ptr_impl<T, Deleter>::reference					reference;
		typedef typename shared_ptr_impl<T, Deleter>::size_type					size_type;

		//���캯��
		constexpr unique_ptr() = default;
		
		explicit unique_ptr(pointer _ptr , Deleter _del = Deleter()) :sPtr_(new shared_ptr_impl(_ptr, _del)) {}

		template <class U>//Ϊ���޶�ֻ�ܴ���ָ��
		explicit unique_ptr(U* _ptr , Deleter _del = Deleter()) :sPtr_(new shared_ptr_impl(_ptr, _del)) {}


		~unique_ptr()
		{
			if (sPtr_.release())
			{
				delete sPtr_;
			}
		}

		//�󲿷�û�����𣬶��ǲ������ⲿ�������������
		//�������ڣ�ֻ��ʹ��ģ�庯��ʱ��ֻ�ܲ���delete�ػ������ܲ���private�ػ���
		
		unique_ptr(const unique_ptr& _rhs) = delete;

		unique_ptr& operator=(const unique_ptr& _rhs) = delete;
		

		//---------------------------------------------------����ӿ�-----------------------------------------------------------
		//����������
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
		//��Ա����
		shared_ptr_impl<T, Deleter>* sPtr_; //����ָ��ĺ��ĺ͹ؼ������ڰ�λ������һ��ָ�룬��ǳ������ָ�롣ÿ�ο�����ָ��ı����ü�������Ϊ++i�� --i���̰߳�ȫ�ģ����Կ����ڼ���Ϊ����������
	};


	//-------------------------------------------------------�ⲿ����-------------------------------------------------------

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







