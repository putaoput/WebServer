//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

//该头文件用来实现unordered_multiset 和 unordered_multiset这两个配接器
#include"hashtable.h"

namespace LT {
	template<class Value,
		class HashFunc = LT::hash<Value>,
		class EqualKey = LT::equal_to<Value>,
		class Alloc = allocator<Value> >
		class unordered_set {
		public:
			//照例进行一些定义
			typedef hashtable<Value, HashFunc, EqualKey, Alloc> container_type;

			typedef typename container_type::key_type             key_type;
			typedef typename container_type::value_type           value_type;
			typedef typename container_type::hasher               hasher;
			typedef typename container_type::key_equal            key_equal;

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
			unordered_set()
				:hashtable_(100, hasher(), EqualKey(), Alloc()) {}
			explicit unordered_set(size_type _n, const HashFunc& _hash = HashFunc(), const EqualKey& _equal = EqualKey(), const Alloc& _alloc = Alloc)
				:hashtable_(_n, _hash, _equal, _alloc) {}
			
			template <class InputIterator,
					typename LT::enable_if<LT::is_input_iterator<InputIterator>::value, int>::type = 0>
			unordered_set(InputIterator _itBegin, InputIterator _itEnd,
				const size_type _n = 100,
				const Hash& _hash = Hash(),
				const KeyEqual& _equal = KeyEqual()
				const Alloc& _alloc = Alloc())
				: hashtable_(LT::max(bucket_count, static_cast<size_type>(LT::distance(_first, last))), hash, equal)
			{
				for (; _first != last; ++_first)
					hashtable_.insert_unique_noresize(*_first);
			}
			unordered_set(std::initializer_list<value_type> ilist,
				const size_type bucket_count = 100,
				const Hash& hash = Hash(),
				const KeyEqual& equal = KeyEqual())
				:hashtable_(LT::max(bucket_count, static_cast<size_type>(ilist.size())), hash, equal)
			{
				for (auto _first = ilist.begin(), last = ilist.end(); _first != last; ++_first)
					hashtable_.insert_unique_noresize(*_first);
			}
			unordered_set(const unordered_set& _rhs)
				:hashtable_(_rhs.hashtable_)
			{
			}
			unordered_set(unordered_set&& _rhs)
				: hashtable_(LT::move(_rhs.hashtable_))
			{
			}

			unordered_set& operator=(const unordered_set& _rhs)
			{
				hashtable_ = _rhs.hashtable_;
				return *this;
			}
			unordered_set& operator=(unordered_set&& _rhs)
			{
				hashtable_ = LT::move(_rhs.hashtable_);
				return *this;
			}
			/*
			unordered_set& operator=(std::initializer_list<value_type> ilist)
			{
				hashtable_.clear();
				hashtable_.reserve(ilist.size());
				for (auto _first = ilist.begin(), last = ilist.end(); _first != last; ++_first)
					hashtable_.insert_unique_noresize(*_first);
				return *this;
			}
			*/

			~unordered_set() {};

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

			void swap(unordered_set& other)
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
			friend bool operator==(const unordered_set& lhs, const unordered_set& rhs)
			{
				return lhs.hashtable_.equal_range_unique(rhs.hashtable_);
			}
			friend bool operator!=(const unordered_set& lhs, const unordered_set& rhs)
			{
				return !lhs.hashtable_.equal_range_unique(rhs.hashtable_);
			}
	};

	// 重载比较操作符
	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator==(const unordered_set<Value, Hash, KeyEqual>& lhs,
		const unordered_set<Value, Hash, KeyEqual>& rhs)
	{
		return lhs == rhs;
	}

	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator!=(const unordered_set<Value, Hash, KeyEqual>& lhs,
		const unordered_set<Value, Hash, KeyEqual>& rhs)
	{
		return lhs != rhs;
	}

	// 重载 swap
	template <class Value, class Hash, class KeyEqual, class Alloc>
	void swap(unordered_set<Value, Hash, KeyEqual>& lhs,
		unordered_set<Value, Hash, KeyEqual>& rhs)
	{
		lhs.swap(rhs);
	}




	template <class Value, 
			  class HashFunc = LT::hash<Value>,
			  class EqualKey = LT::equal_to<Value>, 
			  class Alloc = allocator<Value>>
	class unordered_multiset
	{
	
	public:
		//照例进行一些定义
		typedef hashtable<Value, HashFunc, EqualKey, Alloc> container_type;
		
		typedef typename container_type::key_type             key_type;
		typedef typename container_type::value_type           value_type;
		typedef typename container_type::hasher               hasher;
		typedef typename container_type::key_equal            key_equal;

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
	public:
		//***************************************************************************************
		//*************************************外部接口******************************************
		//***************************************************************************************

		//-----------------------------------构造函数--------------------------------------------
		unordered_multiset()
			:hashtable_(100, hasher(), EqualKey(), Alloc()) {}
		explicit unordered_multiset(size_type _n, const HashFunc& _hash = HashFunc(), const EqualKey& _equal = EqualKey(), const Alloc& _alloc = Alloc)
			:hashtable_(_n, _hash, _equal, _alloc) {}
		
		template <class InputIterator,
				typename LT::enable_if<LT::is_input_iterator<InputIterator>::value,int>::type = 0>
		unordered_multiset(InputIterator _itBegin, InputIterator _itEnd,
			const size_type _n = 100,
			const Hash& _hash = Hash(),
			const KeyEqual& _equal = KeyEqual()
			const Alloc& _alloc = Alloc())
			: hashtable_(LT::max(bucket_count, static_cast<size_type>(LT::distance(_first, last))), hash, equal)
		{
			for (; _first != last; ++_first)
				hashtable_.insert_multi_noresize(*_first);
		}
		unordered_multiset(std::initializer_list<value_type> ilist,
			const size_type bucket_count = 100,
			const Hash& hash = Hash(),
			const KeyEqual& equal = KeyEqual())
			:hashtable_(LT::max(bucket_count, static_cast<size_type>(ilist.size())), hash, equal)
		{
			for (auto _first = ilist.begin(), last = ilist.end(); _first != last; ++_first)
				hashtable_.insert_multi_noresize(*_first);
		}
		unordered_multiset(const unordered_multiset& _rhs)
			:hashtable_(_rhs.hashtable_)
		{
		}
		unordered_multiset(unordered_multiset&& _rhs)
			: hashtable_(LT::move(_rhs.hashtable_))
		{
		}

		unordered_multiset& operator=(const unordered_multiset& _rhs)
		{
			hashtable_ = _rhs.hashtable_;
			return *this;
		}
		unordered_multiset& operator=(unordered_multiset&& _rhs)
		{
			hashtable_ = LT::move(_rhs.hashtable_);
			return *this;
		}
		/*
		unordered_multiset& operator=(std::initializer_list<value_type> ilist)
		{
			hashtable_.clear();
			hashtable_.reserve(ilist.size());
			for (auto _first = ilist.begin(), last = ilist.end(); _first != last; ++_first)
				hashtable_.insert_multi_noresize(*_first);
			return *this;
		}
		*/
		~unordered_multiset() {};

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
		pair<iterator, bool> insert(const value_type & _value)
		{
			return hashtable_.insert_multi(_value);
		}
		pair<iterator, bool> insert(value_type && _value)
		{
			return hashtable_.emplace_multi(LT::move(_value));
		}

		iterator insert(const_iterator _hint, const value_type & _value)
		{
			return hashtable_.insert_multi_use_hint(_hint, _value);
		}
		iterator insert(const_iterator _hint, value_type && _value)
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
		size_type erase(const key_type & _key)
		{
			return hashtable_.erase_multi(_key);
		}

		void clear()
		{
			hashtable_.clear();
		}

		void swap(unordered_multiset & other)
		{
			hashtable_.swap(other.hashtable_);
		}

		// 查找相关
		size_type      count(const key_type & _key) const
		{
			return hashtable_.count(_key);
		}

		iterator       find(const key_type & _key)
		{
			return hashtable_.find(_key);
		}
		const_iterator find(const key_type & _key)  const
		{
			return hashtable_.find(_key);
		}

		pair<iterator, iterator> equal_range(const key_type & _key)
		{
			return hashtable_.equal_range_multi(_key);
		}
		pair<const_iterator, const_iterator> equal_range(const key_type & _key) const
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
		friend bool operator==(const unordered_multiset & lhs, const unordered_multiset & rhs)
		{
			return lhs.hashtable_.equal_range_multi(rhs.hashtable_);
		}
		friend bool operator!=(const unordered_multiset & lhs, const unordered_multiset & rhs)
		{
			return !lhs.hashtable_.equal_range_multi(rhs.hashtable_);
		}
	};

	// 重载比较操作符
	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator==(const unordered_multiset<Value, Hash, KeyEqual>& lhs,
		const unordered_multiset<Value, Hash, KeyEqual>& rhs)
	{
		return lhs == rhs;
	}

	template <class Value, class Hash, class KeyEqual, class Alloc>
	bool operator!=(const unordered_multiset<Value, Hash, KeyEqual>& lhs,
		const unordered_multiset<Value, Hash, KeyEqual>& rhs)
	{
		return lhs != rhs;
	}

	// 重载 swap
	template <class Value, class Hash, class KeyEqual, class Alloc>
	void swap(unordered_multiset<Value, Hash, KeyEqual>& lhs,
		unordered_multiset<Value, Hash, KeyEqual>& rhs)
	{
		lhs.swap(rhs);
	}

} // namespace LT
