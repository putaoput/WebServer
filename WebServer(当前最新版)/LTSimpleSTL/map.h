//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once
//该头文件封装了rb_tree的接口，定义了map和multimap两个模板类

#include "rb_tree.h"

namespace LT {
    // 模板类 map，键值不允许重复
   // 参数一代表键值类型，参数二代表实值类型，参数三代表键值的比较方式，缺省使用 LT::less
    template <class Key,
        class Value,
        class Comp = LT::less<Key>,
        class Alloc = allocator<LT::pair<Key, Value>>>
        class map
    {
    public:
        // map 型别定义
        typedef Key                             key_type;
        typedef Value                           mapped_type;
        typedef LT::pair<const Key, Value>      value_type;
        typedef Comp                            key_compare;

        // 定义一个 functor，用来进行元素比较
        class value_compare : public binary_function <value_type, value_type, bool>
        {
            friend class map<Key, Value, Comp, Alloc>;
        private:
            Comp comp;
            value_compare(Comp _c) : comp(_c) {}
        public:
            bool operator()(const value_type& _lhs, const value_type& _rhs) const
            {
                return comp(_lhs._first, _rhs._first);  // 比较键值的大小
            }
        };

    private:

        typedef LT::rb_tree<value_type, key_compare>  container_type;
        container_type tree_;

    public:
        // 使用 rb_tree 的型别
        typedef typename container_type::node_type              node_type;
        typedef typename container_type::pointer                pointer;
        typedef typename container_type::const_pointer          const_pointer;
        typedef typename container_type::reference              reference;
        typedef typename container_type::const_reference        const_reference;
        typedef typename container_type::iterator               iterator;
        typedef typename container_type::const_iterator         const_iterator;
        typedef typename container_type::reverse_iterator       reverse_iterator;
        typedef typename container_type::const_reverse_iterator const_reverse_iterator;
        typedef typename container_type::size_type              size_type;
        typedef typename container_type::difference_type        difference_type;
        typedef typename container_type::allocator_type         allocator_type;

    public:
        // 构造、复制、移动、赋值函数

        map() = default;

        template <class InputIterator>
        map(InputIterator _first, InputIterator last)
            :tree_()
        {
            tree_.insert_unique(_first, last);
        }

        map(std::initializer_list<value_type> ilist)
            :tree_()
        {
            tree_.insert_unique(ilist.begin(), ilist.end());
        }

        map(const map& _rhs)
            :tree_(_rhs.tree_)
        {
        }
        map(map&& _rhs)
            :tree_(LT::move(_rhs.tree_))
        {
        }

        map& operator=(const map& _rhs)
        {
            tree_ = _rhs.tree_;
            return *this;
        }
        map& operator=(map&& _rhs)
        {
            tree_ = LT::move(_rhs.tree_);
            return *this;
        }

        map& operator=(std::initializer_list<value_type> ilist)
        {
            tree_.clear();
            tree_.insert_unique(ilist.begin(), ilist.end());
            return *this;
        }

        // 相关接口

        key_compare            key_comp()      const { return tree_.key_comp(); }
        value_compare          value_comp()    const { return value_compare(tree_.key_comp()); }
        allocator_type         get_allocator() const { return tree_.get_allocator(); }

        // 迭代器相关

        iterator               begin()
        {
            return tree_.begin();
        }
        const_iterator         begin()   const
        {
            return tree_.begin();
        }
        iterator               end()
        {
            return tree_.end();
        }
        const_iterator         end()     const
        {
            return tree_.end();
        }

        reverse_iterator       rbegin()
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rbegin()  const
        {
            return const_reverse_iterator(end());
        }
        reverse_iterator       rend()
        {
            return reverse_iterator(begin());
        }
        const_reverse_iterator rend()    const
        {
            return const_reverse_iterator(begin());
        }

        const_iterator         cbegin()  const
        {
            return begin();
        }
        const_iterator         cend()    const
        {
            return end();
        }
        const_reverse_iterator crbegin() const
        {
            return rbegin();
        }
        const_reverse_iterator crend()   const
        {
            return rend();
        }

        // 容量相关
        bool                   empty()    const { return tree_.empty(); }
        size_type              size()     const { return tree_.size(); }
        size_type              max_size() const { return tree_.max_size(); }

        // 访问元素相关

        // 若键值不存在，at 会抛出一个异常
        mapped_type& at(const key_type& _key)
        {
            iterator it = lower_bound(_key);
            return it->second;
        }
        const mapped_type& at(const key_type& _key) const
        {
            const_iterator it = lower_bound(_key);
            return it->second;
        }

        mapped_type& operator[](const key_type& _key)
        {
            iterator it = lower_bound(_key);
            if (it == end() || key_comp()(_key, it->_first))
                it = emplace_hint(it, _key, Value{});
            return it->second;
        }
        mapped_type& operator[](key_type&& _key)
        {
            iterator it = lower_bound(_key);

            if (it == end() || key_comp()(_key, it->_first))
                it = emplace_hint(it, LT::move(_key), Value{});
            return it->second;
        }

        // 插入删除相关

        template <class ...Args>
        pair<iterator, bool> emplace(Args&& ..._args)
        {
            return tree_.emplace_unique(LT::forward<Args>(_args)...);
        }

        template <class ...Args>
        iterator emplace_hint(iterator _hint, Args&& ..._args)
        {
            return tree_.emplace_unique_use_hint(_hint, LT::forward<Args>(_args)...);
        }

        pair<iterator, bool> insert(const value_type& _value)
        {
            return tree_.insert_unique(_value);
        }
        pair<iterator, bool> insert(value_type&& _value)
        {
            return tree_.insert_unique(LT::move(_value));
        }

        iterator insert(iterator _hint, const value_type& _value)
        {
            return tree_.insert_unique(_hint, _value);
        }
        iterator insert(iterator _hint, value_type&& _value)
        {
            return tree_.insert_unique(_hint, LT::move(_value));
        }

        template <class InputIterator>
        void insert(InputIterator _first, InputIterator last)
        {
            tree_.insert_unique(_first, last);
        }

        void      erase(iterator _pos) { tree_.erase(_pos); }
        size_type erase(const key_type& _key) { return tree_.erase_unique(_key); }
        void      erase(iterator _first, iterator last) { tree_.erase(_first, last); }

        void      clear() { tree_.clear(); }

        // map 相关操作

        iterator       find(const key_type& _key) { return tree_.find(_key); }
        const_iterator find(const key_type& _key)        const { return tree_.find(_key); }

        size_type      count(const key_type& _key)       const { return tree_.count_unique(_key); }

        iterator       lower_bound(const key_type& _key) { return tree_.lower_bound(_key); }
        const_iterator lower_bound(const key_type& _key) const { return tree_.lower_bound(_key); }

        iterator       upper_bound(const key_type& _key) { return tree_.upper_bound(_key); }
        const_iterator upper_bound(const key_type& _key) const { return tree_.upper_bound(_key); }

        pair<iterator, iterator>
            equal_range(const key_type& _key)
        {
            return tree_.equal_range_unique(_key);
        }

        pair<const_iterator, const_iterator>
            equal_range(const key_type& _key) const
        {
            return tree_.equal_range_unique(_key);
        }

        void           swap(map& _rhs)
        {
            tree_.swap(_rhs.tree_);
        }

    public:
        friend bool operator==(const map& _lhs, const map& _rhs) { return _lhs.tree_ == _rhs.tree_; }
        friend bool operator< (const map& _lhs, const map& _rhs) { return _lhs.tree_ < _rhs.tree_; }
    };

    // 重载比较操作符
    template <class Key, class Value, class Comp, class Alloc>
    bool operator==(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs == _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs < _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator!=(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs == _rhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return _rhs < _lhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<=(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_rhs < _lhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>=(const map<Key, Value, Comp, Alloc>& _lhs, const map<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs < _rhs);
    }

    // 重载 LT 的 swap
    template <class Key, class Value, class Comp, class Alloc>
    void swap(map<Key, Value, Comp, Alloc>& _lhs, map<Key, Value, Comp, Alloc>& _rhs)
    {
        _lhs.swap(_rhs);
    }



    /*****************************************************************************************/

    // 模板类 multimap，键值允许重复
    // 参数一代表键值类型，参数二代表实值类型，参数三代表键值的比较方式，缺省使用 LT::less
    template <class Key,
              class Value,
              class Comp = LT::less<Key>,
              class Alloc = LT::allocator<pair<Key,Value>>>
    class multimap
    {
    public:
        // 照例定义一些类型
        typedef Key                                         key_type;
        typedef Value                                       mapped_type;
        typedef LT::pair<const Key, Value>                  value_type;
        typedef Comp                                        key_compare;
        typedef LT::rb_tree<value_type, key_compare,Alloc>        container_type;
        // 定义一个 functor，用来进行元素比较
        class value_compare : public binary_function <value_type, value_type, bool>
        {
            friend class multimap<Key, Value, Comp,Alloc>;
        private:
            Comp comp;
            value_compare(Comp _c) : comp(_c) {}
        public:
            bool operator()(const value_type& _lhs, const value_type& _rhs) const
            {
                return comp(_lhs._first, _rhs._first);
            }
        };

    private:  
        container_type tree_;

    public:
        // 使用 rb_tree 的型别
        typedef typename container_type::node_type              node_type;
        typedef typename container_type::pointer                pointer;
        typedef typename container_type::const_pointer          const_pointer;
        typedef typename container_type::reference              reference;
        typedef typename container_type::const_reference        const_reference;
        typedef typename container_type::iterator               iterator;
        typedef typename container_type::const_iterator         const_iterator;
        typedef typename container_type::reverse_iterator       reverse_iterator;
        typedef typename container_type::const_reverse_iterator const_reverse_iterator;
        typedef typename container_type::size_type              size_type;
        typedef typename container_type::difference_type        difference_type;
        typedef typename container_type::allocator_type         allocator_type;

    public:
        // 构造、复制、移动函数

        multimap() = default;

        template <class InputIterator>
        multimap(InputIterator _first, InputIterator last)
            :tree_()
        {
            tree_.insert_multi(_first, last);
        }
        multimap(std::initializer_list<value_type> ilist)
            :tree_()
        {
            tree_.insert_multi(ilist.begin(), ilist.end());
        }

        multimap(const multimap& _rhs)
            :tree_(_rhs.tree_)
        {
        }
        multimap(multimap&& _rhs)  
            :tree_(LT::move(_rhs.tree_))
        {
        }

        multimap& operator=(const multimap& _rhs)
        {
            tree_ = _rhs.tree_;
            return *this;
        }
        multimap& operator=(multimap&& _rhs)
        {
            tree_ = LT::move(_rhs.tree_);
            return *this;
        }

        multimap& operator=(std::initializer_list<value_type> ilist)
        {
            tree_.clear();
            tree_.insert_multi(ilist.begin(), ilist.end());
            return *this;
        }

        // 相关接口

        key_compare            key_comp()      const { return tree_.key_comp(); }
        value_compare          value_comp()    const { return value_compare(tree_.key_comp()); }
        allocator_type         get_allocator() const { return tree_.get_allocator(); }

        // 迭代器相关

        iterator               begin()          
        {
            return tree_.begin();
        }
        const_iterator         begin()   const  
        {
            return tree_.begin();
        }
        iterator               end()            
        {
            return tree_.end();
        }
        const_iterator         end()     const  
        {
            return tree_.end();
        }

        reverse_iterator       rbegin()         
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rbegin()  const  
        {
            return const_reverse_iterator(end());
        }
        reverse_iterator       rend()           
        {
            return reverse_iterator(begin());
        }
        const_reverse_iterator rend()    const  
        {
            return const_reverse_iterator(begin());
        }

        const_iterator         cbegin()  const  
        {
            return begin();
        }
        const_iterator         cend()    const  
        {
            return end();
        }
        const_reverse_iterator crbegin() const  
        {
            return rbegin();
        }
        const_reverse_iterator crend()   const  
        {
            return rend();
        }

        // 容量相关
        bool                   empty()    const   { return tree_.empty(); }
        size_type              size()     const   { return tree_.size(); }
        size_type              max_size() const   { return tree_.max_size(); }

        // 插入删除操作

        template <class ...Args>
        iterator emplace(Args&& ..._args)
        {
            return tree_.emplace_multi(LT::forward<Args>(_args)...);
        }

        template <class ...Args>
        iterator emplace_hint(iterator _hint, Args&& ..._args)
        {
            return tree_.emplace_multi_use_hint(_hint, LT::forward<Args>(_args)...);
        }

        iterator insert(const value_type& _value)
        {
            return tree_.insert_multi(_value);
        }
        iterator insert(value_type&& _value)
        {
            return tree_.insert_multi(LT::move(_value));
        }

        iterator insert(iterator _hint, const value_type& _value)
        {
            return tree_.insert_multi(_hint, _value);
        }
        iterator insert(iterator _hint, value_type&& _value)
        {
            return tree_.insert_multi(_hint, LT::move(_value));
        }

        template <class InputIterator>
        void insert(InputIterator _first, InputIterator last)
        {
            tree_.insert_multi(_first, last);
        }

        void           erase(iterator _pos) { tree_.erase(_pos); }
        size_type      erase(const key_type& _key) { return tree_.erase_multi(_key); }
        void           erase(iterator _first, iterator last) { tree_.erase(_first, last); }

        void           clear() { tree_.clear(); }

        // multimap 相关操作

        iterator       find(const key_type& _key) { return tree_.find(_key); }
        const_iterator find(const key_type& _key)        const { return tree_.find(_key); }

        size_type      count(const key_type& _key)       const { return tree_.count_multi(_key); }

        iterator       lower_bound(const key_type& _key) { return tree_.lower_bound(_key); }
        const_iterator lower_bound(const key_type& _key) const { return tree_.lower_bound(_key); }

        iterator       upper_bound(const key_type& _key) { return tree_.upper_bound(_key); }
        const_iterator upper_bound(const key_type& _key) const { return tree_.upper_bound(_key); }

        pair<iterator, iterator>
            equal_range(const key_type& _key)
        {
            return tree_.equal_range_multi(_key);
        }

        pair<const_iterator, const_iterator>
            equal_range(const key_type& _key) const
        {
            return tree_.equal_range_multi(_key);
        }

        void swap(multimap& _rhs)  
        {
            tree_.swap(_rhs.tree_);
        }

    public:
        friend bool operator==(const multimap& _lhs, const multimap& _rhs) { return _lhs.tree_ == _rhs.tree_; }
        friend bool operator< (const multimap& _lhs, const multimap& _rhs) { return _lhs.tree_ < _rhs.tree_; }
    };

    //-------------------------------------------------------外部重载-----------------------------------------------------
    // 重载比较操作符
 template <class Key, class Value, class Comp, class Alloc>
    bool operator==(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return _lhs == _rhs;
    }

 template <class Key, class Value, class Comp, class Alloc>
    bool operator<(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return _lhs < _rhs;
    }

 template <class Key, class Value, class Comp, class Alloc>
    bool operator!=(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return !(_lhs == _rhs);
    }

 template <class Key, class Value, class Comp, class Alloc>
    bool operator>(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return _rhs < _lhs;
    }

 template <class Key, class Value, class Comp, class Alloc>
    bool operator<=(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return !(_rhs < _lhs);
    }

 template <class Key, class Value, class Comp, class Alloc>
    bool operator>=(const multimap<Key, Value, Comp,Alloc>& _lhs, const multimap<Key, Value, Comp,Alloc>& _rhs)
    {
        return !(_lhs < _rhs);
    }

    // 重载 LT 的 swap
 template <class Key, class Value, class Comp, class Alloc>
    void swap(multimap<Key, Value, Comp>& _lhs, multimap<Key, Value, Comp,Alloc>& _rhs)  
    {
        _lhs.swap(_rhs);
    }

}

