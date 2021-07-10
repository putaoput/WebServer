//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once
//��ͷ�ļ���װ��rb_tree�Ľӿڣ�������set��multiset����ģ����

#include "rb_tree.h"

namespace LT {
    // ģ���� set����ֵ�������ظ�
   // ����һ�����ֵ���ͣ�����������ʵֵ���ͣ������������ֵ�ıȽϷ�ʽ��ȱʡʹ�� LT::less
    template <class Key,
        class Value,
        class Comp = LT::less<Key>,
        class Alloc = allocator<LT::pair<Key, Value>>>
        class set
    {
    public:
        // set �ͱ���
        typedef Key                             key_type;
        typedef Value                           mapped_type;
        typedef LT::pair<const Key, Value>      value_type;
        typedef Comp                            key_compare;

        // ����һ�� functor����������Ԫ�رȽ�
        class value_compare : public binary_function <value_type, value_type, bool>
        {
            friend class set<Key, Value, Comp, Alloc>;
        private:
            Comp comp;
            value_compare(Comp _c) : comp(_c) {}
        public:
            bool operator()(const value_type& _lhs, const value_type& _rhs) const
            {
                return comp(_lhs._first, _rhs._first);  // �Ƚϼ�ֵ�Ĵ�С
            }
        };

    private:

        typedef LT::rb_tree<value_type, key_compare>  container_type;
        container_type tree_;

    public:
        // ʹ�� rb_tree ���ͱ�
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
        // ���졢���ơ��ƶ�����ֵ����

        set() = default;

        template <class InputIterator>
        set(InputIterator _first, InputIterator last)
            :tree_()
        {
            tree_.insert_unique(_first, last);
        }

        set(std::initializer_list<value_type> ilist)
            :tree_()
        {
            tree_.insert_unique(ilist.begin(), ilist.end());
        }

        set(const set& _rhs)
            :tree_(_rhs.tree_)
        {
        }
        set(set&& _rhs)
            :tree_(LT::move(_rhs.tree_))
        {
        }

        set& operator=(const set& _rhs)
        {
            tree_ = _rhs.tree_;
            return *this;
        }
        set& operator=(set&& _rhs)
        {
            tree_ = LT::move(_rhs.tree_);
            return *this;
        }

        set& operator=(std::initializer_list<value_type> ilist)
        {
            tree_.clear();
            tree_.insert_unique(ilist.begin(), ilist.end());
            return *this;
        }

        // ��ؽӿ�

        key_compare            key_comp()      const { return tree_.key_comp(); }
        value_compare          value_comp()    const { return value_compare(tree_.key_comp()); }
        allocator_type         get_allocator() const { return tree_.get_allocator(); }

        // ���������

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

        // �������
        bool                   empty()    const { return tree_.empty(); }
        size_type              size()     const { return tree_.size(); }
        size_type              max_size() const { return tree_.max_size(); }

        // ����Ԫ�����

        // ����ֵ�����ڣ�at ���׳�һ���쳣
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

        // ����ɾ�����

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

        // set ��ز���

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

        void           swap(set& _rhs)
        {
            tree_.swap(_rhs.tree_);
        }

    public:
        friend bool operator==(const set& _lhs, const set& _rhs) { return _lhs.tree_ == _rhs.tree_; }
        friend bool operator< (const set& _lhs, const set& _rhs) { return _lhs.tree_ < _rhs.tree_; }
    };

    // ���رȽϲ�����
    template <class Key, class Value, class Comp, class Alloc>
    bool operator==(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs == _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs < _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator!=(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs == _rhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return _rhs < _lhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<=(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_rhs < _lhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>=(const set<Key, Value, Comp, Alloc>& _lhs, const set<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs < _rhs);
    }

    // ���� LT �� swap
    template <class Key, class Value, class Comp, class Alloc>
    void swap(set<Key, Value, Comp, Alloc>& _lhs, set<Key, Value, Comp, Alloc>& _rhs)
    {
        _lhs.swap(_rhs);
    }



    /*****************************************************************************************/

    // ģ���� multiset����ֵ�����ظ�
    // ����һ�����ֵ���ͣ�����������ʵֵ���ͣ������������ֵ�ıȽϷ�ʽ��ȱʡʹ�� LT::less
    template <class Key,
        class Value,
        class Comp = LT::less<Key>,
        class Alloc = LT::allocator<pair<Key, Value>>>
        class multiset
    {
    public:
        // ��������һЩ����
        typedef Key                                         key_type;
        typedef Value                                       mapped_type;
        typedef LT::pair<const Key, Value>                  value_type;
        typedef Comp                                        key_compare;
        typedef LT::rb_tree<value_type, key_compare, Alloc>        container_type;
        // ����һ�� functor����������Ԫ�رȽ�
        class value_compare : public binary_function <value_type, value_type, bool>
        {
            friend class multiset<Key, Value, Comp, Alloc>;
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
        // ʹ�� rb_tree ���ͱ�
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
        // ���졢���ơ��ƶ�����

        multiset() = default;

        template <class InputIterator>
        multiset(InputIterator _first, InputIterator last)
            :tree_()
        {
            tree_.insert_multi(_first, last);
        }
        multiset(std::initializer_list<value_type> ilist)
            :tree_()
        {
            tree_.insert_multi(ilist.begin(), ilist.end());
        }

        multiset(const multiset& _rhs)
            :tree_(_rhs.tree_)
        {
        }
        multiset(multiset&& _rhs)
            :tree_(LT::move(_rhs.tree_))
        {
        }

        multiset& operator=(const multiset& _rhs)
        {
            tree_ = _rhs.tree_;
            return *this;
        }
        multiset& operator=(multiset&& _rhs)
        {
            tree_ = LT::move(_rhs.tree_);
            return *this;
        }

        multiset& operator=(std::initializer_list<value_type> ilist)
        {
            tree_.clear();
            tree_.insert_multi(ilist.begin(), ilist.end());
            return *this;
        }

        // ��ؽӿ�

        key_compare            key_comp()      const { return tree_.key_comp(); }
        value_compare          value_comp()    const { return value_compare(tree_.key_comp()); }
        allocator_type         get_allocator() const { return tree_.get_allocator(); }

        // ���������

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

        // �������
        bool                   empty()    const { return tree_.empty(); }
        size_type              size()     const { return tree_.size(); }
        size_type              max_size() const { return tree_.max_size(); }

        // ����ɾ������

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

        // multiset ��ز���

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

        void swap(multiset& _rhs)
        {
            tree_.swap(_rhs.tree_);
        }

    public:
        friend bool operator==(const multiset& _lhs, const multiset& _rhs) { return _lhs.tree_ == _rhs.tree_; }
        friend bool operator< (const multiset& _lhs, const multiset& _rhs) { return _lhs.tree_ < _rhs.tree_; }
    };

    //-------------------------------------------------------�ⲿ����-----------------------------------------------------
    // ���رȽϲ�����
    template <class Key, class Value, class Comp, class Alloc>
    bool operator==(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs == _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return _lhs < _rhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator!=(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs == _rhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return _rhs < _lhs;
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator<=(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_rhs < _lhs);
    }

    template <class Key, class Value, class Comp, class Alloc>
    bool operator>=(const multiset<Key, Value, Comp, Alloc>& _lhs, const multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        return !(_lhs < _rhs);
    }

    // ���� LT �� swap
    template <class Key, class Value, class Comp, class Alloc>
    void swap(multiset<Key, Value, Comp>& _lhs, multiset<Key, Value, Comp, Alloc>& _rhs)
    {
        _lhs.swap(_rhs);
    }

}

