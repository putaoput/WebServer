//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

//该头文件用来实现unordered_map 和 unordered_multimap这两个配接器
#include "hashtable.h"
#include "algobase.h"
namespace LT {
	template<class Value,
		class HashFunc = LT::hash<Value>,
		class EqualKey = LT::equal_to<Value>,
		class Alloc = allocator<Value> >
		class unordered_map {
		public:
			//照例进行一些定义
			typedef hashtable<Value, HashFunc, EqualKey, Alloc>	 container_type;

			typedef typename container_type::key_type             key_type;
			typedef typename container_type::value_type           value_type;
			typedef typename container_type::hasher               hasher;
			typedef typename container_type::key_equal            key_equal;
			typedef typename container_type::mapped_type		  mapped_type;

			typedef typename container_type::size_type            size_type;
			typedef typename container_type::difference_type      difference_type;
			typedef typename container_type::pointer              pointer;
			typedef typename container_type::const_pointer        const_pointer;
			typedef typename container_type::reference            reference;
			typedef typename container_type::const_reference      const_reference;

			typedef typename container_type::const_iterator       iterator;
			typedef typename container_type::const_iterator       const_iterator;

			hasher    hash_fcn()               const { return hashtable_.hash_fcn(); }
			key_equal key_eq()                 const { return hashtable_.key_eq(); }
		private:
			//成员变量
			container_type hashtable_;

		public:
			//***************************************************************************************
			//*************************************外部接口******************************************
			//***************************************************************************************

			//-----------------------------------构造函数--------------------------------------------
			unordered_map()
				:hashtable_(100, hasher(), EqualKey(), Alloc()) {}
			explicit unordered_map(size_type _n, const HashFunc& _hash = HashFunc(), const EqualKey& _equal = EqualKey(), const Alloc& _alloc = Alloc)
				:hashtable_(_n, _hash, _equal, _alloc) {}
			
			template <class InputIterator,
				typename LT::enable_if<is_input_iterator<InputIterator>::value, int>::type = 0>
			unordered_map(InputIterator _itBeg, InputIterator _itEnd,
				const size_type _n = 100,
				const HashFunc& _hash = HashFunc(),
				const EqualKey& _equal = EqualKey(),
				const Alloc& _alloc = Alloc())
				: hashtable_(LT::max(_n, static_cast<size_type>(LT::distance(_itBeg, _itEnd))), _hash, _equal)
			{
				for (; _itBeg != _itEnd; ++_itBeg)
				{
					hashtable_.insert_unique_noresize(*++_itBeg);
				}
					
			}
			
			
			unordered_map(std::initializer_list<value_type> _ilist,
				const size_type _n = 100,
				const HashFunc& _hash = HashFunc(),
				const EqualKey& _equal = EqualKey(),
				const Alloc& _alloc = Alloc())
				:hashtable_(LT::max(_n, static_cast<size_type>(_ilist.size())), _hash, _equal)
			{
				for (auto cur = _ilist.begin(); cur != _ilist.end(); ++cur)
					hashtable_.insert_unique_noresize(*cur);
			}
			
			unordered_map(const unordered_map& _rhs)
				:hashtable_(_rhs.hashtable_){}
			unordered_map(unordered_map&& _rhs)
				: hashtable_(LT::move(_rhs.hashtable_))
			{

			}

			unordered_map& operator=(const unordered_map& _rhs)
			{
				hashtable_ = _rhs.hashtable_;
				return *this;
			}
			unordered_map& operator=(unordered_map&& _rhs)
			{
				hashtable_ = LT::move(_rhs.hashtable_);
				return *this;
			}
			
			unordered_map& operator=(std::initializer_list<value_type> _ilist)
			{
				hashtable_.clear();
				hashtable_.reserve(_ilist.size());
				for (auto _first = _ilist.begin(), last = _ilist.end(); _first != last; ++_first)
					hashtable_.insert_unique_noresize(*_first);
				return *this;
			}
			

			~unordered_map() {};

			//--------------------------------对外接口------------------------------------------

			// 迭代器相关

			iterator begin()
			{
				return hashtable_.begin();
			}
			const_iterator begin()  const
			{
				return hashtable_.begin();
			}
			iterator end()
			{
				return hashtable_.end();
			}
			const_iterator end()  const
			{
				return hashtable_.end();
			}

			const_iterator cbegin() const
			{
				return hashtable_.cbegin();
			}
			const_iterator cend() const
			{
				return hashtable_.cend();
			}

			// 容量相关

			bool  empty() const { return hashtable_.empty(); }
			size_type size()  const { return hashtable_.size(); }
			size_type max_size() const { return hashtable_.max_size(); }

			size_type bucket_count() const
			{
				return hashtable_.bucket_count();
			}
			size_type max_bucket_count() const
			{
				return hashtable_.max_bucket_count();
			}
			size_type bucket_size(size_type n) const
			{
				return hashtable_.bucket_size(n);
			}
			size_type bucket(const key_type& _key) const
			{
				return hashtable_.bucket(_key);
			}

			// 修改容器操作

			//emplace
			template <class ...Args>
			pair<iterator, bool> emplace(Args&& ..._args)
			{
				return hashtable_.emplace_unique(LT::forward<Args>(_args)...);
			}

			template <class ...Args>
			iterator emplace_hint(const_iterator _hint, Args&& ..._args)
			{
				return hashtable_.emplace_unique_use_hint(_hint, LT::forward<Args>(_args)...);
			}

			// insert
			pair<iterator, bool> insert(const value_type& _value)
			{
				return hashtable_.insert_unique(_value);
			}
			pair<iterator, bool> insert(value_type&& _value)
			{
				return hashtable_.emplace_unique(LT::move(_value));
			}

			iterator insert(const_iterator _hint, const value_type& _value)
			{
				return hashtable_.insert_unique_use_hint(_hint, _value);
			}
			iterator insert(const_iterator _hint, value_type&& _value)
			{
				return hashtable_.emplace_unique_use_hint(_hint, LT::move(_value));
			}

			template <class InputIterator>
			void insert(InputIterator _first, InputIterator last)
			{
				hashtable_.insert_unique(_first, last);
			}

			// erase 
			void erase(iterator it)
			{
				hashtable_.erase(it);
			}
			void erase(iterator _first, iterator last)
			{
				hashtable_.erase(_first, last);
			}

			size_type erase(const key_type& _key)
			{
				return hashtable_.erase_unique(_key);
			}

			void clear()
			{
				hashtable_.clear();
			}

			void swap(unordered_map& other)
			{
				hashtable_.swap(other.hashtable_);
			}

			// 查找相关
			size_type      count(const key_type& _key) const
			{
				return hashtable_.count(_key);
			}

			iterator       find(const key_type& _key)
			{
				return hashtable_.find(_key);
			}
			const_iterator find(const key_type& _key)  const
			{
				return hashtable_.find(_key);
			}

			pair<iterator, iterator> equal_range(const key_type& _key)
			{
				return hashtable_.equal_range_unique(_key);
			}
			pair<const_iterator, const_iterator> equal_range(const key_type& _key) const
			{
				return hashtable_.equal_range_unique(_key);
			}

			//--------------------------------------------------------------容量配置器相关-----------------------------------------
			Alloc get_allocator() const { return Alloc(); }

			//哈希表相关

			// 获取负载系数
			float load_factor() const
			{
				return hashtable_.load_factor();
			}
			//获取最大负载系数
			float max_load_factor()const
			{
				return hashtable_.max_load_factor();
			}
			//设置最大负载系数
			void max_load_factor(float _newFactor) {
				hashtable_.max_load_factor(_newFactor);
			}
			
			void      rehash(size_type count) { hashtable_.rehash(count); }
			void      reserve(size_type count) { hashtable_.reserve(count); }

		public:
			friend bool operator==(const unordered_map& _lhs, const unordered_map& _rhs)
			{
				return _lhs.hashtable_.equal_range_unique(_rhs.hashtable_);
			}
			friend bool operator!=(const unordered_map& _lhs, const unordered_map& _rhs)
			{
				return !_lhs.hashtable_.equal_range_unique(_rhs.hashtable_);
			}
	};

	// 重载比较操作符
	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator==(const unordered_map<Value, Hash, KeyEqual>& _lhs,
		const unordered_map<Value, Hash, KeyEqual>& _rhs)
	{
		return _lhs == _rhs;
	}

	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator!=(const unordered_map<Value, Hash, KeyEqual>& _lhs,
		const unordered_map<Value, Hash, KeyEqual>& _rhs)
	{
		return _lhs != _rhs;
	}

	// 重载 swap
	template <class Value, class Hash, class KeyEqual, class Alloc>
	void swap(unordered_map<Value, Hash, KeyEqual>& _lhs,
		unordered_map<Value, Hash, KeyEqual>& _rhs)
	{
		_lhs.swap(_rhs);
	}




	template <class Value,
			class HashFunc = LT::hash<Value>,
			class EqualKey = LT::equal_to<Value>,
			class Alloc = allocator<Value>>
		class unordered_multimap
	{

	public:
		//照例进行一些定义
		typedef hashtable<Value, HashFunc, EqualKey, Alloc> container_type;

		typedef typename container_type::key_type             key_type;
		typedef typename container_type::value_type           value_type;
		typedef typename container_type::hasher               hasher;
		typedef typename container_type::key_equal            key_equal;
		typedef typename container_type::mapped_type		  mapped_type;

		typedef typename container_type::size_type            size_type;
		typedef typename container_type::difference_type      difference_type;
		typedef typename container_type::pointer              pointer;
		typedef typename container_type::const_pointer        const_pointer;
		typedef typename container_type::reference            reference;
		typedef typename container_type::const_reference      const_reference;

		typedef typename container_type::const_iterator       iterator;
		typedef typename container_type::const_iterator       const_iterator;


	private:
		//数据成员
		container_type hashtable_;
	public:
		//***************************************************************************************
		//*************************************外部接口******************************************
		//***************************************************************************************

		//-----------------------------------构造函数--------------------------------------------
		unordered_multimap()
			:hashtable_(100, hasher(), EqualKey(), Alloc()) {}
		explicit unordered_multimap(size_type _n, const HashFunc& _hash = HashFunc(), const EqualKey& _equal = EqualKey(), const Alloc& _alloc = Alloc)
			:hashtable_(_n, _hash, _equal, _alloc) {}
		
		template <class InputIterator,
				typename LT::enable_if<LT::is_input_iterator<InputIterator>::value, int>::type = 0>
		unordered_multimap(InputIterator _itBegin, InputIterator _itEnd,
			const size_type _n = 100,
			const HashFunc& _hash = HashFunc(),
			const EqualKey& _equal = EqualKey(),
			const Alloc& _alloc = Alloc())
			: hashtable_(LT::max(_n, static_cast<size_type>(LT::distance(_itBegin, _itEnd))), _hash, _equal,_alloc)
		{
			for (; _itBegin != _itEnd; ++_itBegin)
				hashtable_.insert_multi_noresize(*_itBegin);
		}
		
		unordered_multimap(std::initializer_list<value_type> _ilist,
			const size_type _n = 100,
			const HashFunc& _hash = HashFunc(),
			const EqualKey& _equal = EqualKey(),
			const Alloc& _alloc = Alloc())
			: hashtable_(LT::max(_n, _ilist.size()), _hash, _equal, _alloc)
		{
			for (value_type val : _ilist)
			{
				hashtable_.insert_multi_noresize(val);
			}
				
		}
		
		unordered_multimap(const unordered_multimap& _rhs)
			:hashtable_(_rhs.hashtable_){}

		unordered_multimap(unordered_multimap&& _rhs)
			: hashtable_(LT::move(_rhs.hashtable_)){}

		unordered_multimap& operator=(const unordered_multimap& _rhs)
		{
			hashtable_ = _rhs.hashtable_;
			return *this;
		}
		unordered_multimap& operator=(unordered_multimap&& _rhs)
		{
			hashtable_ = LT::move(_rhs.hashtable_);
			return *this;
		}
		
		unordered_multimap& operator=(std::initializer_list<value_type> _ilist)
		{
			hashtable_.clear();
			hashtable_.reserve(_ilist.size());
			for (value_type val : _ilist)
			{
				hashtable_.insert_multi_noresize(val);
			}
				
			return *this;
		}
		
		~unordered_multimap() {};

		//--------------------------------对外接口------------------------------------------

		// 迭代器相关

		iterator begin()
		{
			return hashtable_.begin();
		}
		const_iterator begin()  const
		{
			return hashtable_.begin();
		}
		iterator end()
		{
			return hashtable_.end();
		}
		const_iterator end()  const
		{
			return hashtable_.end();
		}

		const_iterator cbegin() const
		{
			return hashtable_.cbegin();
		}
		const_iterator cend() const
		{
			return hashtable_.cend();
		}

		// 容量相关

		bool  empty() const { return hashtable_.empty(); }
		size_type size()  const { return hashtable_.size(); }
		size_type max_size() const { return hashtable_.max_size(); }

		size_type bucket_count() const
		{
			return hashtable_.bucket_count();
		}
		size_type max_bucket_count() const
		{
			return hashtable_.max_bucket_count();
		}
		size_type bucket_size(size_type n) const
		{
			return hashtable_.bucket_size(n);
		}
		size_type bucket(const key_type& _key) const
		{
			return hashtable_.bucket(_key);
		}

		// 修改容器操作

		//emplace
		template <class ...Args>
		pair<iterator, bool> emplace(Args&& ..._args)
		{
			return hashtable_.emplace_multi(LT::forward<Args>(_args)...);
		}

		template <class ...Args>
		iterator emplace_hint(const_iterator _hint, Args&& ..._args)
		{
			return hashtable_.emplace_multi_use_hint(_hint, LT::forward<Args>(_args)...);
		}

		// insert
		pair<iterator, bool> insert(const value_type& _value)
		{
			return hashtable_.insert_multi(_value);
		}
		pair<iterator, bool> insert(value_type&& _value)
		{
			return hashtable_.emplace_multi(LT::move(_value));
		}

		iterator insert(const_iterator _hint, const value_type& _value)
		{
			return hashtable_.insert_multi_use_hint(_hint, _value);
		}
		iterator insert(const_iterator _hint, value_type&& _value)
		{
			return hashtable_.emplace_multi_use_hint(_hint, LT::move(_value));
		}

		template <class InputIterator>
		void insert(InputIterator _first, InputIterator last)
		{
			hashtable_.insert_multi(_first, last);
		}

		// erase 
		void erase(iterator it)
		{
			hashtable_.erase(it);
		}
		void erase(iterator _first, iterator last)
		{
			hashtable_.erase(_first, last);
		}
		size_type erase(const key_type& _key)
		{
			return hashtable_.erase_multi(_key);
		}

		void clear()
		{
			hashtable_.clear();
		}

		void swap(unordered_multimap& other)
		{
			hashtable_.swap(other.hashtable_);
		}

		// 查找相关
		size_type      count(const key_type& _key) const
		{
			return hashtable_.count(_key);
		}

		iterator       find(const key_type& _key)
		{
			return hashtable_.find(_key);
		}
		const_iterator find(const key_type& _key)  const
		{
			return hashtable_.find(_key);
		}

		pair<iterator, iterator> equal_range(const key_type& _key)
		{
			return hashtable_.equal_range_multi(_key);
		}
		pair<const_iterator, const_iterator> equal_range(const key_type& _key) const
		{
			return hashtable_.equal_range_multi(_key);
		}
		//--------------------------------------------------------------容量配置器相关-----------------------------------------
		Alloc get_allocator() const { return Alloc(); }

		//哈希表相关

		//获取负载系数
		float load_factor() const
		{
			return hashtable_.load_factor();
		}
		//获取最大负载系数
		float max_load_factor()const
		{
			return hashtable_.max_load_factor();
		}
		//设置最大负载系数
		void max_load_factor(float _newFactor) {
			hashtable_.max_load_factor(_newFactor);
		}

		void      rehash(size_type count) { hashtable_.rehash(count); }
		void      reserve(size_type count) { hashtable_.reserve(count); }

	public:
		friend bool operator==(const unordered_multimap& _lhs, const unordered_multimap& _rhs)
		{
			return _lhs.hashtable_.equal_range_multi(_rhs.hashtable_);
		}
		friend bool operator!=(const unordered_multimap& _lhs, const unordered_multimap& _rhs)
		{
			return !_lhs.hashtable_.equal_range_multi(_rhs.hashtable_);
		}
	};

	// 重载比较操作符
	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator==(const unordered_multimap<Value, Hash, KeyEqual>& _lhs,
		const unordered_multimap<Value, Hash, KeyEqual>& _rhs)
	{
		return _lhs == _rhs;
	}

	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator!=(const unordered_multimap<Value, Hash, KeyEqual>& _lhs,
		const unordered_multimap<Value, Hash, KeyEqual>& _rhs)
	{
		return _lhs != _rhs;
	}

	// 重载 swap
	template <class Value, class Hash, class KeyEqual, class Alloc>
	void swap(unordered_multimap<Value, Hash, KeyEqual>& _lhs,
		unordered_multimap<Value, Hash, KeyEqual>& _rhs)
	{
		_lhs.swap(_rhs);
	}

} // namespace LT
