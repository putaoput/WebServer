//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once

#include"functional.h"
#include "type_traits.h"
#include "iterator.h"
#include "allocator.h"
#include "assert.h"
#include "stl_algo.h"
//#include "algobase.h"
#include "memory.h"
#include <vector>//自己写的vector还没调试完成

//该头文件负责提供基本数据结构:哈希表。以模板的形式定义
//！！！从实现hashtable开始，学习并使用了桥接模式这一设计模式
namespace LT {

	//定义节点
	template <class T>
	struct __hashtable_node {
		__hashtable_node* next_; //开链法需要
		T                 data_;//回到篮子 

		__hashtable_node() = default;
		__hashtable_node(const T& _value) :next_(nullptr), data_(_value) {}

		__hashtable_node(const __hashtable_node& _node) :next_(_node.next_), data_(_node.data_) {}
		__hashtable_node(__hashtable_node&& _node) :next_(_node.next_), data_(LT::move(_node.data_))
		{
			_node.next_ = nullptr;
		}
	};

	//*********************************************************桥接模式：取值接口实现**************************************************
	//实现的原理是写两个模板，一个pair，一个非pair，然后__hashtable_value_traits使用is_pair来完成判断，然后调用对应模板的定义和函数
	//其实这样写挺麻烦的，但是也是一种思路，因为无论如何，总要有一层完成pair和非pair的分割
	// 获取类型
	template <class T, bool>
	struct __hashtable_value_traits_impl
	{
		typedef T key_type;
		typedef T mapped_type;
		typedef T value_type;

		template <class Ty>
		static const key_type& get_key(const Ty& _value)
		{
			return _value;
		}

		template <class Ty>
		static const value_type& get_value(const Ty& _value)
		{
			return _value;
		}
	};

	template <class T>
	struct __hashtable_value_traits_impl<T, true>
	{
		typedef typename LT::remove_cv<typename T::first_type>::type		key_type;
		typedef typename T::second_type										mapped_type;
		typedef T															value_type;

		template <class Ty>
		static const key_type& get_key(const Ty& _value)
		{
			return _value._first;
		}

		template <class Ty>
		static const value_type& get_value(const Ty& _value)
		{
			return _value;
		}
	};

	template <class T>//T可以是pair类型
	struct __hashtable_value_traits
	{
		static constexpr bool is_map = is_pair<T>::_value;//如果输入的T是pair类型，那么将返回true_type，否则返回false_type

		typedef __hashtable_value_traits_impl<T, is_map>   value_traits_type;//用来记录存的是pair还是set

		typedef typename value_traits_type::key_type    key_type;
		typedef typename value_traits_type::mapped_type mapped_type;
		typedef typename value_traits_type::value_type  value_type;

		template <class Ty>
		static const key_type& get_key(const Ty& _value)
		{
			return value_traits_type::get_key(_value);
		}

		template <class Ty>
		static const value_type& get_value(const Ty& _value)
		{
			return value_traits_type::get_value(_value);
		}
	};
		//*******************************************************************************************************************
		template <class T, class HashFunc, class EqualKey,class Alloc = allocator<T>>
		class hashtable;

		template <class T, class HashFunc, class EqualKey, class Alloc>
		struct __hashtable_iterator;

		template <class T, class HashFunc, class EqualKey, class Alloc>
		struct __hashtable_const_iterator;

		template <class T>
		struct __hashtable_local_iterator;

		template <class T>
		struct __hashtable_const_local_iterator;

		//********************************************迭代器实现******************************************************
		template <class T, class HashFunc, class EqualKey, class Alloc >
		struct __hashtable_iterator_base :public LT::iterator<LT::forward_iterator_tag, T>
		{
			//照例进行一些定义
			typedef LT::hashtable<T, HashFunc, EqualKey,Alloc>					   hashtable;
			typedef __hashtable_iterator_base<T, HashFunc, EqualKey,Alloc>		   base;
			typedef __hashtable_iterator<T, HashFunc, EqualKey,Alloc>              iterator;
			typedef __hashtable_const_iterator<T, HashFunc, EqualKey,Alloc>        const_iterator;
			typedef __hashtable_node<T>*											node_ptr;
			typedef hashtable*   												   hashtable_ptr;
			typedef const node_ptr										       const_node_ptr;
			typedef const hashtable_ptr									       const_hashtable_ptr;

			typedef size_t												     size_type;
			typedef ptrdiff_t											     difference_type;

			node_ptr    node_;  // 迭代器当前所指节点
			hashtable_ptr hashtablePtr_;    // 迭代器所在的哈希表

			__hashtable_iterator_base() = default;
			__hashtable_iterator_base(node_ptr _node, hashtable_ptr _hashtable):node_(_node), hashtablePtr_(_hashtable){}

			bool operator==(const base& _rhs) const { return node_ == _rhs.node_; }
			bool operator!=(const base& _rhs) const { return node_ != _rhs.node_; }
		};

		template <class T, class HashFunc, class EqualKey, class Alloc >
		struct __hashtable_iterator :public __hashtable_iterator_base<T,HashFunc,EqualKey,Alloc>
		{
			//照例进行一些定义
			typedef __hashtable_iterator_base<T, HashFunc, EqualKey,Alloc>		base;
			typedef typename base::hashtable					hashtable;
			typedef typename base::iterator						iterator;
			typedef typename base::const_iterator				const_iterator;
			typedef typename base::node_ptr						node_ptr;
			typedef typename base::hashtable_ptr				hashtable_ptr;

			typedef __hashtable_value_traits<T>					value_traits;
			typedef T											value_type;
			typedef value_type*									pointer;
			typedef value_type&									reference;

			using base::node_;
			using base::hashtablePtr_;

			__hashtable_iterator() = default;
			__hashtable_iterator(node_ptr _node, hashtable_ptr _hashtable):__hashtable_iterator_base(_node,_hashtable){}
			__hashtable_iterator(const iterator& _rhs)
			{
				node_ = _rhs.node_;
				hashtablePtr_ = _rhs.hashtablePtr_;
			}
			__hashtable_iterator(const const_iterator& _rhs)
			{
				node_ = _rhs.node_;
				hashtablePtr_ = _rhs.hashtablePtr_;
			}
			iterator& operator=(const iterator& _rhs)
			{
				if (this != &_rhs)
				{
					node_ = _rhs.node_;
					hashtablePtr_ = _rhs.hashtablePtr_;
				}
				return *this;
			}
			iterator& operator=(const const_iterator& _rhs)
			{
				if (this != &_rhs)
				{
					node_ = _rhs.node_;
					hashtablePtr_ = _rhs.hashtablePtr_;
				}
				return *this;
			}

			//迭代器的基本操作符，提供*，->, ++,++(int)
			reference operator*()  const { return node_->data_; }
			pointer   operator->() const { return &(operator*()); }

			iterator& operator++()
			{
				assert(node_);
				const node_ptr old = node_;
				node_ = node_->next_;
				if (!node_)
				{  //转到下一个bucket的开始
					auto index = hashtablePtr_->hash(value_traits::get_key(old->data_));
					while (!node_ && ++index < hashtablePtr_->bucket_size_)
						node_ = hashtablePtr_->buckets_[index];
				}
				return *this;
			}
			iterator operator++(int)
			{
				iterator tmp = *this;
				++* this;
				return tmp;
			}
		};

		template <class T, class HashFunc, class EqualKey, class Alloc >
		struct __hashtable_const_iterator :public __hashtable_iterator_base<T, HashFunc, EqualKey, Alloc>
		{
			//照例进行一些定义
			typedef __hashtable_iterator_base<T, HashFunc, EqualKey,Alloc>		base;

			typedef typename base::hashtable					hashtable;
			typedef typename base::iterator						iterator;
			typedef typename base::const_iterator				const_iterator;
			typedef typename base::node_ptr						node_ptr;
			typedef typename base::hashtable_ptr				hashtable_ptr;

			typedef __hashtable_value_traits<T>					value_traits;										
			typedef T											value_type;
			typedef value_type* pointer;
			typedef value_type& reference;

			//成员变量
			node_ptr node_;
			hashtable_ptr hashtablePtr_;

			//构造函数
			__hashtable_const_iterator() = default;
			__hashtable_const_iterator(node_ptr _node, hashtable_ptr _hashtable) :__hashtable_iterator_base(_node, _hashtable) {}
			__hashtable_const_iterator(const iterator& _rhs)
			{
				node_ = _rhs.node_;
				hashtablePtr_ = _rhs.hashtablePtr_;
			}
			__hashtable_const_iterator(const const_iterator& _rhs)
			{
				node_ = _rhs.node_;
				hashtablePtr_ = _rhs.hashtablePtr_;
			}
			const_iterator& operator=(const iterator& _rhs)
			{
				if (this != &_rhs)
				{
					node_ = _rhs.node_;
					hashtablePtr_ = _rhs.hashtablePtr_;
				}
				return *this;
			}
			const_iterator& operator=(const const_iterator& _rhs)
			{
				if (this != &_rhs)
				{
					node_ = _rhs.node_;
					hashtablePtr_ = _rhs.hashtablePtr_;
				}
				return *this;
			}

			//迭代器的基本操作符，提供*，->, ++,++(int)
			reference operator*()  const { return node_->_value; }
			pointer   operator->() const { return &(operator*()); }

			const_iterator& operator++()
			{
				assert(node_);
				const node_ptr old = node_;
				node_ = node_->next_;
				if (!node_)
				{  //转到下一个bucket的开始
					auto index = hashtablePtr_->hash(value_traits::get_key(old->data_));
					while (!node_ && ++index < hashtablePtr_->bucket_size_)
						node_ = hashtablePtr_->buckets_[index];
				}
				return *this;
			}
			const_iterator operator++(int)
			{
				iterator tmp = *this;
				++* this;
				return tmp;
			}
		};
		//*******************************************

		// bucket大小定义

#if (_MSC_VER && _WIN64) || ((__GNUC__ || __clang__) &&__SIZEOF_POINTER__ == 8)
#define SYSTEM_64 1
#else
#define SYSTEM_32 1
#endif

#ifdef SYSTEM_64

#define PRIME_NUM 99

// 1. start with p = 101
// 2. p = next_prime(p * 1.7)
// 3. if p < (2 << 63), go to step 2, otherwise, go to step 4
// 4. end with p = prev_prime(2 << 63 - 1)
		static constexpr size_t hashtable_prime_list[] = {
		  101ull, 173ull, 263ull, 397ull, 599ull, 907ull, 1361ull, 2053ull, 3083ull,
		  4637ull, 6959ull, 10453ull, 15683ull, 23531ull, 35311ull, 52967ull, 79451ull,
		  119179ull, 178781ull, 268189ull, 402299ull, 603457ull, 905189ull, 1357787ull,
		  2036687ull, 3055043ull, 4582577ull, 6873871ull, 10310819ull, 15466229ull,
		  23199347ull, 34799021ull, 52198537ull, 78297827ull, 117446801ull, 176170229ull,
		  264255353ull, 396383041ull, 594574583ull, 891861923ull, 1337792887ull,
		  2006689337ull, 3010034021ull, 4515051137ull, 6772576709ull, 10158865069ull,
		  15238297621ull, 22857446471ull, 34286169707ull, 51429254599ull, 77143881917ull,
		  115715822899ull, 173573734363ull, 260360601547ull, 390540902329ull,
		  585811353559ull, 878717030339ull, 1318075545511ull, 1977113318311ull,
		  2965669977497ull, 4448504966249ull, 6672757449409ull, 10009136174239ull,
		  15013704261371ull, 22520556392057ull, 33780834588157ull, 50671251882247ull,
		  76006877823377ull, 114010316735089ull, 171015475102649ull, 256523212653977ull,
		  384784818980971ull, 577177228471507ull, 865765842707309ull, 1298648764060979ull,
		  1947973146091477ull, 2921959719137273ull, 4382939578705967ull, 6574409368058969ull,
		  9861614052088471ull, 14792421078132871ull, 22188631617199337ull, 33282947425799017ull,
		  49924421138698549ull, 74886631708047827ull, 112329947562071807ull, 168494921343107851ull,
		  252742382014661767ull, 379113573021992729ull, 568670359532989111ull, 853005539299483657ull,
		  1279508308949225477ull, 1919262463423838231ull, 2878893695135757317ull, 4318340542703636011ull,
		  6477510814055453699ull, 9716266221083181299ull, 14574399331624771603ull, 18446744073709551557ull
		};

#else

#define PRIME_NUM 44

// 1. start with p = 101
// 2. p = next_prime(p * 1.7)
// 3. if p < (2 << 31), go to step 2, otherwise, go to step 4
// 4. end with p = prev_prime(2 << 31 - 1)
		static constexpr size_t hashtable_prime_list[] = {
		  101u, 173u, 263u, 397u, 599u, 907u, 1361u, 2053u, 3083u, 4637u, 6959u,
		  10453u, 15683u, 23531u, 35311u, 52967u, 79451u, 119179u, 178781u, 268189u,
		  402299u, 603457u, 905189u, 1357787u, 2036687u, 3055043u, 4582577u, 6873871u,
		  10310819u, 15466229u, 23199347u, 34799021u, 52198537u, 78297827u, 117446801u,
		  176170229u, 264255353u, 396383041u, 594574583u, 891861923u, 1337792887u,
		  2006689337u, 3010034021u, 4294967291u,
		};

#endif

		//*******************************************获取质数*****************************************************
		inline size_t hashtable_next_prime(size_t _n) {
			const size_t* _first = hashtable_prime_list;
			const size_t* last = hashtable_prime_list + PRIME_NUM;
			const size_t* pos = LT::lower_bound(_first, last, _n);
			return pos == last ? *(last - 1) : *pos;
		}

		//*******************************************hashtable的实现********************************************************
		template<class T, class HashFunc, class EqualKey, class Alloc = allocator<T>>
		class hashtable
		{
			friend struct __hashtable_iterator<T, HashFunc, EqualKey, Alloc>;
			friend struct __hashtable_const_iterator<T, HashFunc, EqualKey, Alloc>;
		public:
			//照例进行一些定义
			//类型
			typedef __hashtable_value_traits<T>                 value_traits;
			typedef typename value_traits::key_type             key_type;
			typedef typename value_traits::mapped_type          mapped_type;
			typedef typename value_traits::value_type           value_type;
			typedef typename T* pointer;
			typedef typename const T* const_pointer;
			typedef typename T& reference;
			typedef typename const T& const_reference;
			typedef typename size_t					            size_type;
			typedef typename ptrdiff_t					        difference_type;
			typedef HashFunc                                    hasher;
			typedef EqualKey                                    key_equal;

			//哈希节点
			typedef __hashtable_node<T>                         node;
			typedef node* node_ptr;
			

			//迭代器
			typedef __hashtable_iterator<T, HashFunc, EqualKey, Alloc>				iterator;
			typedef __hashtable_const_iterator<T, HashFunc, EqualKey, Alloc>		const_iterator;

			//哈希表的成员变量
		private:
			hasher					hash_;
			key_equal				equals_;
			std::vector<node_ptr>	buckets_;
			size_type				elementsNum_;
			float					maxLoadFactor_;//最大负载系数

		//*****************************************************************************************************************************************************
		//************************************************************************对外接口*********************************************************************
		//*****************************************************************************************************************************************************

			//------------------------------------------构造和析构-------------------------------------------------------
			//构造函数
			explicit hashtable(size_type _n, const HashFunc& _hf = HashFunc(), const EqualKey& _eq = EqualKey())
				:hash_(_hf), equals_(_eq), elementsNum_(0), maxLoadFactor_(1.0)
			{
				__init(_n);
			}

			
			template<
					class InputIter,
					typename std::enable_if<LT::is_input_iterator<InputIter>::_value, int>::type = 0
					>
			hashtable(InputIter _itBegin, InputIter _itEnd, size_type _n, const HashFunc& _hf = HashFunc(), const EqualKey& _eq = EqualKey())
				: hash_(_hf), 
				equals_(_eq),
				elementsNum_(LT::distance(_itBegin, _itEnd)),
				maxLoadFactor_(1.0)
			{
				__init(_itBegin, _itEnd);
			}
			
			hashtable(const hashtable& _rhs)
				:hash_(_rhs.hash_),
				 equals_(_rhs.equals_),
				 elementsNum_(_rhs.elementsNum_),
				 maxLoadFactor_(1.0)
			{
				__init(_rhs);
			}

			hashtable(hashtable&& _rhs)
				:elementsNum_ (_rhs.elementsNum_),
				hash_ (_rhs.hash_),
				equals_ (_rhs.equals_),
				maxLoadFactor_(1.0),
				buckets_ (LT::move(_rhs.buckets_))
			{	
				_rhs.elementsNum = 0;
			}

			hashtable& operator=(const hashtable& _rhs) {
				elementsNum_ = _rhs.elementsNum_;
				hash_ = _rhs.hash_;
				equals_ = _rhs.equals_;
				maxLoadFactor_ = _rhs.maxLoadFactor_();
				__init(_rhs);
			};
			hashtable& operator=(hashtable&& _rhs) {
				elementsNum_ = _rhs.elementsNum_;
				hash_ = _rhs.hash_;
				equals_ = _rhs.equals_;
				maxLoadFactor_ = _rhs.maxLoadFactor_();
				buckets_ = LT::move(_rhs.buckets_);
				_rhs.elementsNum_ = 0;

			}

			~hashtable() { clear(); }

			//-----------------------------------------迭代器相关-------------------------------------------------------------
			iterator begin() { return __begin(); }
			const_iterator begin() { return __cbegin(); }
			iterator end() { return  iterator(nullptr, this); }
			const_iterator end() { return const_iterator(nullptr, const_cast<hashtable*>(this)); }

			const_iterator cbegin() { return __cbegin(); }
			const_iterator cend() { return const_iterator(nullptr, const_cast<hashtable*>(this)); }

			//-----------------------------------------容量相关----------------------------------------------------------------------
			bool			empty()					{ return !elementsNum_; }
			size_type		size()					{ return elementsNum_; }
			size_type		max_size()				{ return static_cast<size_type>(-1); }
			void			resize(size_type _n)	{ rehash(_n);}

			size_type		bucket_count() const	{ return buckets_.size(); }
			size_type		max_bucket_count()		{ return hashtable_prime_list[PRIME_NUM - 1]; }

			//某个bucket里面的节点数
			size_type		bucket_size(size_type _bucketsIdx) 
			{
				size_type ret = 0;
				for (node_ptr cur = buckets_[_bucketsIdx]; cur; cur = cur->next_) {
					++ret;
				}
				return ret;
			}
			size_type		bucket(const key_type& _key)    const { return hash_(_key); }

			//----------------------------------------元素相关-----------------------------------------‘

			//放置元素不允许重复 
			template<class ...Args>
			pair<iterator, bool>	emplace_unique(Args&& ..._args) 
			{
				resize(elementsNum_ + 1);
				node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
				pair<iterator, bool> ret = __insert_node_unique(newNode);
				if (!ret.second) {//如果插入失败了，要把这里事先新建的节点析构掉
					__destruct_one_node(newNode);
				}
				return ret;
			}

			//放置元素允许重复 
			template<class ...Args>
			iterator				emplace_multi(Args&& ..._args) {
				resize(elementsNum_ + 1);
				node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
				return __insert_node_multi(newNode);
			}

			template<class ...Args>
			iterator				emplace_unique_use_hint(const_iterator, Args&&... _args) {}

			template<class ...Args>
			iterator emplace_multi_use_hint(const_iterator, Args&&... _args) {}

			//无需扩充hashtable即可插入
			iterator insert_multi_noresize(const value_type& _value) {
				const size_type bucketsIdx = hash_(get_key(_value));
				node_ptr nodeList = buckets_[bucketsIdx];
				node_ptr newNode = __create_one_node(_value, nullptr);
				//分成两种情况，一种是hashtable里面已经存在该键值，一种是不存在
				for (node_ptr cur = nodeList; cur; cur = cur->next_) {
					if (equals_(get_key(cur->data_), get_key(_value))) {
						newNode->next_ = cur->next_;
						cur->next_ = newNode;
						++elementsNum_;
						return iterator(newNode, this);
					}
				}

				//新插入的元素放置在头部
				newNode->next_ = nodeList;
				buckets_[bucketsIdx] = newNode;
				++elementsNum_;
				return iterator(newNode, this);
			};
			pair<iterator, bool> insert_unique_noresize(const value_type& _value) {
				const size_type bucketsIdx = hash_(get_key(_value));
				node_ptr nodeList = buckets_[bucketsIdx];
				for (node_ptr cur = nodeList; cur; cur = cur->next_) {
					if (equals_(get_key(cur->data_), get_key(_value))) {
						//插入失败
						return LT::make_pair(iterator(cur, this), false);
					}
				}
				//新插入的元素放置在头部
				node_ptr newNode = __create_one_node(_value, nullptr);
				newNode->next_ = nodeList;
				buckets_[bucketsIdx] = newNode;
				++elementsNum_;
				return iterator(newNode, this);
			}

			iterator insert_multi(const value_type& _value)
			{
				resize(elementsNum_ + 1);
				return insert_multi_noresize(_value);
			}
			iterator insert_multi(value_type&& _value)
			{
				return emplace_multi(LT::move(_value));
			}

			pair<iterator, bool> insert_unique(const value_type& _value)
			{
				resize(elementsNum_ + 1);//如果需要，将会再哈希
				return insert_unique_noresize(_value);//然后插入
			}
			pair<iterator, bool> insert_unique(value_type&& _value)
			{
				return emplace_unique(LT::move(_value));
			}


			iterator insert_multi_use_hint(const_iterator /*_hint*/, const value_type& _value)
			{
				return insert_multi(_value);
			}
			iterator insert_multi_use_hint(const_iterator /*_hint*/, value_type&& _value)
			{
				return emplace_multi(LT::move(_value));
			}

			iterator insert_unique_use_hint(const_iterator /*_hint*/, const value_type& _value)
			{
				return insert_unique(_value)._first;
			}
			iterator insert_unique_use_hint(const_iterator /*_hint*/, value_type&& _value)
			{
				return emplace_unique(LT::move(_value));
			}

			template <class InputIter>
			void insert_multi(InputIter _first, InputIter last)
			{
				copy_insert_multi(_first, last, iterator_category(_first));
			}

			template <class InputIter>
			void insert_unique(InputIter _first, InputIter last)
			{
				copy_insert_unique(_first, last, iterator_category(_first));
			}


			void erase(const_iterator _position) { __erase(_position); }
			void  erase(const_iterator _itBeg, const_iterator _itEnd) { __erase(_itBeg, _itEnd); }

			size_type erase_multi(const key_type& _key) { __erase_multi(_key); }
			size_type erase_unique(const key_type& _key) { __erase_multi(_key); }

			void      clear() {
				__destruct(buckets_);
				elementsNum_ = 0;
			}

			void      swap(hashtable& _rhs) {
				if (this == _rhs) { return; }
				buckets_.swap(_rhs.buckets_);
				LT::swap(elementsNum_, _rhs.elementsNum_);
				LT::swap(hash_, _rhs.hash_);
				LT::swap(equals_, _rhs.equals_);
			};

			// 查找相关操作

			size_type count(const key_type& _key) const
			{
				const size_type bucketsIdx = __hash(_key);
				size_type size = 0;
				for (node_ptr cur = buckets_[bucketsIdx]; cur; cur = cur->next_) {
					if (equals_(__get_key(cur->data_), _key)) {
						++size;
					}
				}
				return size;
			};

			iterator                             find(const key_type& _key)
			{
				const size_type bucketsIdx = __hash(_key);
				node_ptr nodeList = buckets_[bucketsIdx];
				for (; nodeList && !equals_(__get_key(nodeList->data_), _key); nodeList = nodeList->next_) {}
				return iterator(nodeList, this);
			}
			const_iterator                       find(const key_type& _key) const
			{
				const size_type bucketsIdx = __hash(_key);
					node_ptr nodeList = buckets_[bucketsIdx];
					for (; nodeList && !equals_(__get_key(nodeList->data_), _key); nodeList = nodeList->next_) {}
				return const_iterator(static_cast<node_ptr>(nodeList), static_cast<hashtable*>(this));
			}

			pair<iterator, iterator>             equal_range_multi(const key_type& _key)
			{
				__equal_range_multi(_key);
			}
			pair<const_iterator, const_iterator> equal_range_multi(const key_type& _key) const
			{
				__equal_range_multi(_key);
			}

			pair<iterator, iterator>             equal_range_unique(const key_type& _key)
			{
				__equal_range_unique(_key);
			}
			pair<const_iterator, const_iterator> equal_range_unique(const key_type& _key) const
			{
				__equal_range_unique(_key);
			}


			//--------------------------------------------------------------容量配置器相关-----------------------------------------
			Alloc get_allocator() const { return Alloc(); }

			//负载系数
			float load_factor()const
			{
				return static_cast<float>(elementsNum_) / static_cast<float>(bucket_size.size());
			}
			float max_load_factor()const
			{
				return maxLoadFactor_;
			}
			void max_load_factor(float _newMaxLoadFactor)
			{
				maxLoadFactor_ = _newMaxLoadFactor;
			}

			void rehash(size_type _n) {
				__rehash(_n);
			}

			//将哈希表中桶的数量设置为最适合包含至少n个元素的桶数
			void reserve(size_type _Maxcount)
			{
				rehash(static_cast<size_type>(static_cast<float>(_Maxcount) / max_load_factor() + 0.5F));
			}

			hasher    hash_fcn() const { return hash_; }
			key_equal key_eq()   const { return equals_; }
			
			//*****************************************************************************************************************************************************
			//************************************************************************内部实现*********************************************************************
			//*****************************************************************************************************************************************************
		private:
			//内存分配
			//typedef static_cast<node_ptr>(Alloc::allocate(sizeof(node)))		node_allocator ;
			typedef allocator<node>												node_allocator;
			typedef allocator<T>												data_allocator;


			//配置一个节点的内存
			inline node_ptr __allocate_one_node_mem() {
				return node_allocator::allocate();
			}

			//初始化一个节点的内存的空间
			inline void __construct_one_node(node_ptr _ptr, const value_type& _value = value_type(), node_ptr _next = nullptr) {
				LT::construct(LT::address_of(_ptr->data_), _value);
			}

			template<class T, class ...Args>
			void __construct_one_node(node_ptr _ptr, Args... _args) {
				LT::construct(LT::address_of(_ptr->data_), LT::forward<Args>(_args)...);//逗号方式展开参数包
				_ptr->next_ = nullptr;
			}


			//一步创建好一个node,该函数不提供异常保证，交给上层函数
			inline node_ptr __create_one_node(const value_type& _value, node_ptr _preNode = nullptr) {
				node_ptr node = __allocate_one_node_mem();
				__construct_one_node(node, _value);
				if (_preNode) {
					_preNode->next_ = node;
				}
				return node;
			}

			template <class ...Args>
			node_ptr __create_one_node(Args&& ..._args) {
				node_ptr newNode = __allocate_one_node_mem();
				try {
					__construct_one_node(newNode, LT::forward<Args>(_args)...);

				}
				catch (...) {
					__deallocte_one_node_mem(newNode);
				}
				return newNode;

			}
			//析构一个节点
			inline void __destroy_one_node(node_ptr _ptr) {
				LT::destroy(LT::address_of(_ptr->data_));
			}

			//回收一个节点的内存
			inline node_ptr __deallocte_one_node_mem(node_ptr _ptr) {
				//在这里返回一个nullptr，为的就是方便的避免指针空悬。
				node_allocator::deallocate(_ptr);
			}

			//一步回收好一个node
			inline node_ptr __destruct_one_node(node_ptr _ptr, node_ptr _preNode = nullptr) {
				if (_preNode) {
					_preNode->next_ = _ptr->next_;
				}
				__destroy_one_node(_ptr);
				__deallocte_one_node_mem(_ptr);
			}
			//一次回收完成一个buckets数组的所有东西
			inline void __destruct(std::vector<node_ptr>& _invalidBk) {
				for (node_ptr& ptr : _invalidBk) {
					while (ptr) {
						node_ptr toDestruct = ptr;
						ptr = ptr->next_;
						__destruct_one_node(toDestruct);
					}
				}
			}
			//初始化

			inline void __init(size_type _n) {
				const auto bucketNum = hashtable_next_prime(_n);
				buckets_.reserve(bucketNum);
				bucket.assign(bucketNum, nullptr);
			}

			template<class InputIter>
			inline void __init(InputIter _itBegin, InputIter _itEnd) {
				//由于无法获知是否允许重复，所以只能先给一个大小，具体还有一个一个由上层配接器插入新元素。
				size_type bucketsSize = hashtable_next_prime(elementsNum_);
				buckets_.resize(bucketsSize, nullptr);		
			}
			inline void __init(hashtable& _rhs) {
				size_type bucketsSize = _rhs.buckest_.size();
				buckets_.reserve(bucketsSize);
				buckets_.assign(bucketsSize, nullptr);
				try
				{
					size_type pos = 0;
					for (node_ptr nodeList : _rhs.buckets_) {
						if (nodeList) {//需要深拷贝这个节点里面的所有内容
							node_ptr preNode = nullptr;
							do {
								node_ptr newNode = __create_one_node(*nodeList, preNode);
								preNode = newNode;
								nodeList = nodeList->next_;
							} while (nodeList);
						}
					}
				}
				catch (...)
				{
					clear();
				}
			}


			//-------------------------------------------------------------------------------接口实现-------------------------------------------------------------
			//迭代器：

			//begin
			iterator __begin() 
			{
				for (node_ptr nodeList : buckets_) {
					if (nodeList) {
						return iterator(nodeList, this);
					}
				}
				return iterator(nullptr, this);
			}

			const_iterator __cbegin()
			{
				for (node_ptr nodeList : buckets_) {
					if (nodeList) {
						return const_iterator(nodeList, const_cast<hashtable*>(this));
					}
				}
				return const_iterator(nullptr, const_cast<hashtable*>(this));
			}


			//插入insert,在新建好节点之后，提供插入功能，注意如果返回了false，析构节点的工作要由上一层调用该函数的函数负责
			inline iterator __insert_node_multi(node_ptr _node)
			{
				const size_type bucketsIdx = hash_(value_traits::get_key(_node->data_));
				node_ptr nodeList = buckets_[bucketsIdx];
				for (node_ptr cur = nodeList; cur; cur = cur->next_) {
					if (is_equal(__get_key(_node->data_), __get_key(cur->data_))) {
						_node->next_ = cur;
						cur->next_ = _node;
						++elementsNum_;
						return iterator(_node, this);
					}
				}
				//说明需要插入
				_node->next_ = nodeList;
				buckets_[bucketsIdx] = _node;
				++elementsNum_;
				return iterator(_node, this);
			}

			inline pair<iterator, bool> __insert_node_unique(node_ptr _node) 
			{
				const size_type bucketsIdx = __hash(__get_key(_node->data_));
				node_ptr nodeList = buckets_[bucketsIdx];
				for (node_ptr cur = nodeList; cur; cur = cur->next_) {
					if (is_equal(__get_key(_node->data_), __get_key(cur->data_))) {
						return LT::make_pair(iterator(cur, this), false);
					}
				}
				//说明需要插入
				_node->next_ = nodeList;
				buckets_[bucketsIdx] = _node;
				++elementsNum_;
				return LT::make_pair(iterator(_node, this), true);
			}

			//范围查找:
			inline pair<iterator, iterator> __equal_range_multi(const key_type& _key)
			{
				const size_type bucketsIdx = __hash(_key);
				for (node_ptr cur = buckets_[bucketsIdx]; cur; cur = cur->next_) {
					if (equals_(__get_key(cur->data_), _key)) {
						//找到了第一个相同的节点，要接着查找下去。
						node_ptr endNode = cur->next_;
						for (; endNode && equals_(__get_key(endNode->data_), _key); endNode){}
						if (endNode == nullptr) {
							for (size_type idx = bucketsIdx + 1; idx < buckets_.size(); ++idx) {
								if (buckets_[idx]) {
									endNode = buckets_[idx];
									while (endNode && equals_(__get_key(endNode->data_, _key))) {
										endNode = endNode->next_;
									}
									if (endNode) {
										break;
									}
								}
							}
						}
						return LT::make_pair(iterator(cur, this), iterator(endNode, this));
					}
				}
				//没有查找到该元素
				return LT::make_pair(iterator(nullptr, *this), iterator(nullptr, *this));
			}

			inline pair<iterator, iterator> __equal_range_multi(const key_type& _key) const
			{ 
				const size_type bucketsIdx = __hash(_key);
				for (node_ptr cur = buckets_[bucketsIdx]; cur; cur = cur->next_) {
					if (equals_(__get_key(cur->data_), _key)) {
						//找到了第一个相同的节点，要接着查找下去。
						node_ptr endNode = cur->next_;
						for (; endNode && equals_(__get_key(endNode->data_), _key); endNode) {}
						if (endNode == nullptr) {
							for (size_type idx = bucketsIdx + 1; idx < buckets_.size(); ++idx) {
								if (buckets_[idx]) {
									endNode = buckets_[idx];
									while (endNode && equals_(__get_key(endNode->data_, _key))) {
										endNode = endNode->next_;
									}
									if (endNode) {
										break;
									}
								}
							}
						}
						return LT::make_pair(const_iterator(cur, const_cast<hashtable*>(this)), const_iterator(endNode, const_cast<hashtable*>(this)));
					}
				}
				//没有查找到该元素
				
				return LT::make_pair(cend(), cend());
			}

			inline pair<iterator, iterator> __equal_range_unique(const key_type& _key)
			{
				const size_type bucketsIdx = __hash(_key);
				for (node_ptr cur = buckets_[bucketsIdx]; cur; cur = cur->next_) {
					if (equals_(__get_key(cur->data_), _key)) {
						//找到了,下一步是找到这个节点的下一个节点
						node_ptr endNode = cur->next_;
						if (endNode == nullptr) {
							for (size_type idx = bucketsIdx + 1; idx < buckets_.size(); ++idx) {
								if (buckets_[idx]) {
									endNode = buckets_[idx];
									break;
								}
							}
						}
						return LT::make_pair(iterator(cur, this), iterator(endNode, this));
					}
				}
				//没有查找到该元素
				return LT::make_pair(iterator(nullptr, *this), iterator(nullptr, *this));
			}

			inline pair<iterator, iterator> __equal_range_unique(const key_type& _key) const
			{
				const size_type bucketsIdx = __hash(_key);
				for (node_ptr cur = buckets_[bucketsIdx]; cur; cur = cur->next_) {
					if (equals_(__get_key(cur->data_), _key)) {
						//找到了,下一步是找到这个节点的下一个节点
						node_ptr endNode = cur->next_;
						if (endNode == nullptr) {
							for (size_type idx = bucketsIdx + 1; idx < buckets_.size(); ++idx) {
								if (buckets_[idx]) {
									endNode = buckets_[idx];
									break;
								}
							}
						}
						return LT::make_pair(const_iterator(cur, const_cast<hashtable*>(this)), iterator(endNode, const_cast<hashtable*>(this)));
					}

				}
				//没有查找到该元素
				return LT::make_pair(cend(), cend()); 
			}
			//--删除操作
			inline void __erase(const_iterator _positon) {
				//删除该迭代器对应元素
				node_ptr node = _positon.node_;
				if (node) {
					//说明这不是空迭代器，也避免了是end() / cend()迭代器
					const size_type bucketsIdx = __hash(node->data_);
					if (buckets_[bucketsIdx] == nullptr) { return; }
					if (buckets_[bucketsIdx] == node) {
						buckets_[bucketsIdx] = node->next_;
					}
					node_ptr nodeList = buckets_[bucketsIdx];
					while (nodeList->next_) {
						if (nodeList->next_ == node) {
							__destruct_one_node(nodeList->next_, nodeList);
						}
						nodeList = nodeList->next_;
					}
				}
			}

			//给定哪一个bucketsIdx，然后查找删除区间内的节点，默认删到尾
			void __erase_from_one_bucket(size_type _bucketsIdx, node_ptr _ptrBeg, node_ptr _ptrEnd = nullptr) {
				node_ptr toDelete = buckets_[_bucketsIdx];
				node_ptr nextNode = toDelete->next_;
				if (buckets_[_bucketsIdx] == _ptrBeg) {
					//说明从头开始删除
					node_ptr nextNode = toDelete->next_;
					for (; nextNode; nextNode = nextNode->next_) {
						__destruct_one_node(toDelete);
						if (nextNode == _ptrEnd)
						{
							break;
						}
						toDelete = nextNode;
					}
					buckets_[_bucketsIdx] = nextNode;
				}
				else {
					//不需要从头开始删除
					node_ptr preNode = toDelete;
					toDelete = nextNode;
					for (; toDelete; toDelete = toDelete->next_) {
						if (toDelete == _ptrBeg) {
							break;
						}
						preNode = toDelete;
					}
					nextNode = toDelete->next_;
					for (; nextNode; nextNode = nextNode->next_) {
						__destruct_one_node(toDelete);
						if (nextNode == _ptrEnd)
						{
							break;
						}
						toDelete = nextNode;
					}
					preNode->next_ = nextNode;
				}
					
			}

			inline void __erase(const_iterator _itBeg, const_iterator _itEnd) {
				node_ptr begNode = _itBeg.node_;
				node_ptr endNode = _itEnd.node_;
				if (begNode == endNode) { return; }

				const size_type bucketsIdx = __hash(begNode->data_);
				if (buckets_[bucketsIdx] == nullptr) { return; }
			
				//删除bucketsIdx这个桶内需要删除的链表
				__erase_from_one_bucket(bucketsIdx, begNode, endNode);
				const size_type endIdx = endNode ? __hash(endNode->data_) : buckets_.size();
				
				for (size_type i = bucketsIdx + 1; i < endIdx; ++i) {
					__erase_from_one_bucket(i, buckets_[i], endNode);
				}
			}
			
			//实际上，下面两个erase，特别是第一个equal是可以不调用equal_range_...进行加速的，特别是第二个。
			inline size_type _erase_multi(const key_type& _key) {
				pair<const_iterator, const_iterator> itPair = equal_range_multi(_key);
				size_type ret = LT::advance(itPair._first,itPair.second);
				if (itPair._first != itPair.second) {
					erase(itPair._first, itPair.second);
				}
				return ret;
			}

			inline size_type _erase_unique(const key_type& _key) {
				pair<const_iterator, const_iterator> itPair = equal_range_unique(_key);
				if (itPair._first != itPair.second) {
					erase(itPair._first, itPair.second);
					return 0;
				}
				return 0;
			}

			//-------------------------------------------------hashtable独有的一些辅助函数------------------------------------------------------
			//hash
			size_type __hash(const key_type& _key, size_type _n = 0)const{
				return  _n ? hash_(_key) % _n : hash_(_key) % buckets_.size();
			}

			inline void __rehash(size_type _n) {
				size_type bucketsSize = hashtable_next_prime(_n);
				size_type maxSize = buckets_.size() * maxLoadFactor_;
				if (bucketsSize > maxSize) {
					const size_type newSize = hashtable_next_prime(_n);
					if (newSize > maxSize) {
						std::vector<node_ptr, Alloc> newBuckets(newSize,nullptr);
						try{
							//方法就是逐个访问原有哈希表中的元素，然后插入新的哈希表
							size_type old_n = elementsNum_;
							for (size_type bucketsIdx = 0; bucketsIdx < old_n; ++bucketsIdx) {
								node_ptr nodeList = buckets_[bucketsIdx];
								while (nodeList) {
									size_type newIdx = __hash(nodeList->data_, newBuckets.size());
									buckets_[bucketsIdx] = nodeList->next_;
									nodeList->next_ = newBuckets[newIdx];
									newBuckets[newIdx] = nodeList;
									nodeList = buckets_[bucketsIdx];
								}
							}
							buckets_.swap(newBuckets);
							__destruct(newBuckets);
						}
						catch (...) {
							__destruct(newBuckets);
						}

					}
				}
			}

			//get_key
			size_type __get_key(const value_type& _value)const {
				return value_traits::get_key(_value);
			}
		};

		//外部重载
		//swap
		template<class T, class HashFunc, class EqualKey, class Alloc>
		void swap(hashtable<T, HashFunc, EqualKey, Alloc>& _lhs, hashtable<T, HashFunc, EqualKey, Alloc>& _rhs)
		{
			_lhs.swap(_rhs);
		}
}