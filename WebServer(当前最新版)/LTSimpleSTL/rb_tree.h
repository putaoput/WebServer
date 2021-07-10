//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//��ͷ�ļ������ṩ�������ݽṹ:���������ģ�����ʽ����
//�ֱ���Ҫ��pair��ȡ��������ڵ���ƣ��������������ƣ��������Ƽ�������
//��ͷ�ļ��ĵ���������㣬������㷨ʵ�־��ο���Դ���룬���������ƿ�ܲο���Դ���룬�����ڲ�����ʵ���Ƕ������
//��֤�˴���ṹ����ȷ�������ڲ�ʵ��������п��ǲ���֮��

#include <initializer_list>
#include <cassert>

#include "functional.h"
#include "iterator.h"
#include "memory.h"
#include "type_traits.h"
#include "allocator.h"

namespace LT 
{
	//--------------------------����Ҫ���������ڵ���ɫ-------------------------------------------
	typedef bool rb_tree_color_type;
	static constexpr rb_tree_color_type red_node = false;
	static constexpr rb_tree_color_type black_node = true;


    //---------------------------------------forward declaration----------------------------------

    template <class T> struct rb_tree_node_base;
    template <class T> struct rb_tree_node;

    template <class T> struct rb_tree_iterator;
    template <class T> struct rb_tree_const_iterator;

    //----------------------------------�����������ȡ---------------------------------------------------

    //��pair����
    template <class T, bool>
    struct rb_tree_value_traits_impl
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

    //��pair����
    template <class T>
    struct rb_tree_value_traits_impl<T, true>
    {
        typedef typename std::remove_cv<typename T::first_type>::type key_type;
        typedef typename T::second_type                               mapped_type;
        typedef T                                                     value_type;

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

    template <class T>
    struct rb_tree_value_traits
    {
        static constexpr bool is_map = LT::is_pair<T>::_value;

        typedef rb_tree_value_traits_impl<T, is_map> value_traits_type;

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

    // rb tree node_ traits

    template <class T>
    struct rb_tree_node_traits
    {
        typedef rb_tree_color_type                 color_type;

        typedef rb_tree_value_traits<T>            value_traits;
        typedef typename value_traits::key_type    key_type;
        typedef typename value_traits::mapped_type mapped_type;
        typedef typename value_traits::value_type  value_type;

        typedef rb_tree_node_base<T>* base_ptr;
        typedef rb_tree_node<T>* node_ptr;
    };

    // --------------------------------rb tree �Ľڵ����--------------------------------

    template <class T>
    struct rb_tree_node_base
    {
        typedef rb_tree_color_type          color_type;
        typedef rb_tree_node_base<T>*       base_ptr;
        typedef rb_tree_node<T>*            node_ptr;

        base_ptr   parent;  // ���ڵ�
        base_ptr   left;    // ���ӽڵ�
        base_ptr   right;   // ���ӽڵ�
        color_type color;   // �ڵ���ɫ

        base_ptr get_base_ptr()
        {
            return &*this;
        }

        node_ptr get_node_ptr()
        {
            return reinterpret_cast<node_ptr>(&*this);
        }

        node_ptr& get_node_ref()
        {
            return reinterpret_cast<node_ptr&>(*this);
        }
    };

    template <class T>
    struct rb_tree_node :public rb_tree_node_base<T>
    {
        typedef rb_tree_node_base<T>* base_ptr;
        typedef rb_tree_node<T>* node_ptr;

        T value_;  // �ڵ�ֵ

        base_ptr get_base_ptr()
        {
            return static_cast<base_ptr>(&*this);
        }

        node_ptr get_node_ptr()
        {
            return &*this;
        }
    };

    // ------------------------------------------------rb tree traits----------------------------------------------------

    template <class T>
    struct rb_tree_traits
    {
        typedef rb_tree_value_traits<T>            value_traits;

        typedef typename value_traits::key_type    key_type;
        typedef typename value_traits::mapped_type mapped_type;
        typedef typename value_traits::value_type  value_type;

        typedef value_type* pointer;
        typedef value_type& reference;
        typedef const value_type* const_pointer;
        typedef const value_type& const_reference;

        typedef rb_tree_node_base<T>               base_type;
        typedef rb_tree_node<T>                    node_type;

        typedef base_type*                         base_ptr;
        typedef node_type*                         node_ptr;
    };

    // --------------------------------------rb tree �ĵ�����-------------------------------------------------

    template <class T>
    struct rb_tree_iterator_base :public LT::iterator<LT::bidirectional_iterator_tag, T>
    {
        typedef typename rb_tree_traits<T>::base_ptr  base_ptr;

        base_ptr node_;  // ָ��ڵ㱾��

        rb_tree_iterator_base() :node_(nullptr) {}

        // ʹ������ǰ��
        void inc()
        {
            if (node_->right != nullptr)
            {
                node_ = rb_tree_min(node_->right);
            }
            else
            {  // ���û�����ӽڵ�
                auto y = node_->parent;
                while (y->right == node_)
                {
                    node_ = y;
                    y = y->parent;
                }
                if (node_->right != y)  // Ӧ�ԡ�Ѱ�Ҹ��ڵ����һ�ڵ㣬�����ڵ�û�����ӽڵ㡱���������
                    node_ = y;
            }
        }

        // ʹ����������
        void dec()
        {
            if (node_->parent->parent == node_ && rb_tree_is_red(node_))
            { // ��� node_ Ϊ header
                node_ = node_->right;  // ָ���������� max �ڵ�
            }
            else if (node_->left != nullptr)
            {
                node_ = rb_tree_max(node_->left);
            }
            else
            {  // �� header �ڵ㣬Ҳ�����ӽڵ�
                auto y = node_->parent;
                while (node_ == y->left)
                {
                    node_ = y;
                    y = y->parent;
                }
                node_ = y;
            }
        }

        bool operator==(const rb_tree_iterator_base& _rhs) { return node_ == _rhs.node_; }
        bool operator!=(const rb_tree_iterator_base& _rhs) { return node_ != _rhs.node_; }
    };

    template <class T>
    struct rb_tree_iterator :public rb_tree_iterator_base<T>
    {
        typedef rb_tree_traits<T>                tree_traits;

        typedef typename tree_traits::value_type value_type;
        typedef typename tree_traits::pointer    pointer;
        typedef typename tree_traits::reference  reference;
        typedef typename tree_traits::base_ptr   base_ptr;
        typedef typename tree_traits::node_ptr   node_ptr;

        typedef rb_tree_iterator<T>              iterator;
        typedef rb_tree_const_iterator<T>        const_iterator;
        typedef iterator                         self;

        using rb_tree_iterator_base<T>::node_;

        // ���캯��
        rb_tree_iterator() {}
        rb_tree_iterator(base_ptr _x) { node_ = _x; }
        rb_tree_iterator(node_ptr _x) { node_ = _x; }
        rb_tree_iterator(const iterator& _rhs) { node_ = _rhs.node_; }
        rb_tree_iterator(const const_iterator& _rhs) { node_ = _rhs.node_; }

        // ���ز�����
        reference operator*()  const { return node_->get_node_ptr()->_value; }
        pointer   operator->() const { return &(operator*()); }

        self& operator++()
        {
            this->inc();
            return *this;
        }
        self operator++(int)
        {
            self tmp(*this);
            this->inc();
            return tmp;
        }
        self& operator--()
        {
            this->dec();
            return *this;
        }
        self operator--(int)
        {
            self tmp(*this);
            this->dec();
            return tmp;
        }
    };


    template <class T>
    struct rb_tree_const_iterator :public rb_tree_iterator_base<T>
    {
        typedef rb_tree_traits<T>                     tree_traits;

        typedef typename tree_traits::value_type      value_type;
        typedef typename tree_traits::const_pointer   pointer;
        typedef typename tree_traits::const_reference reference;
        typedef typename tree_traits::base_ptr        base_ptr;
        typedef typename tree_traits::node_ptr        node_ptr;

        typedef rb_tree_iterator<T>                   iterator;
        typedef rb_tree_const_iterator<T>             const_iterator;
        typedef const_iterator                        self;

        using rb_tree_iterator_base<T>::node_;

        // ���캯��
        rb_tree_const_iterator() {}
        rb_tree_const_iterator(base_ptr _x) { node_ = _x; }
        rb_tree_const_iterator(node_ptr _x) { node_ = _x; }
        rb_tree_const_iterator(const iterator& _rhs) { node_ = _rhs.node_; }
        rb_tree_const_iterator(const const_iterator& _rhs) { node_ = _rhs.node_; }

        // ���ز�����
        reference operator*()  const { return node_->get_node_ptr()->value; }
        pointer   operator->() const { return &(operator*()); }

        self& operator++()
        {
            this->inc();
            return *this;
        }
        self operator++(int)
        {
            self tmp(*this);
            this->inc();
            return tmp;
        }
        self& operator--()
        {
            this->dec();
            return *this;
        }
        self operator--(int)
        {
            self tmp(*this);
            this->dec();
            return tmp;
        }
    };

    //----------------------------------------------------������㷨------------------------------------------------------------

    //Ѱ����С�ڵ�ֵ
    template <class NodePtr>
    NodePtr rb_tree_min(NodePtr _x)  
    {
        while (_x->left != nullptr)
        {
            _x = _x->left;
        }
            
        return _x;
    }

    //Ѱ�����ڵ�ֵ
    template <class NodePtr>
    NodePtr rb_tree_max(NodePtr _x)  
    {
        while (_x->right != nullptr)
        {
            _x = _x->right;
        }
            
        return _x;
    }

    //�жϵ�ǰ�ڵ��ǲ���һ�������
    template <class NodePtr>
    bool rb_tree_is_lchild(NodePtr _node)  
    {
        return _node == _node->parent->left;
    }

    //�жϵ�ǰ�ڵ��ǲ��Ǻ�ɫ
    template <class NodePtr>
    bool rb_tree_is_red(NodePtr _node)  
    {
        return _node->color == red_node;
    }

    //�ѵ�ǰ�ڵ���Ϊ��ɫ
    template <class NodePtr>
    void rb_tree_set_black(NodePtr& _node)  
    {
        _node->color = black_node;
    }

    //�ѵ�ǰ�ڵ���Ϊ��ɫ
    template <class NodePtr>
    void rb_tree_set_red(NodePtr& _node)  
    {
        _node->color = red_node;
    }

    //���ص�ǰ�ڵ����һ���ڵ�
    template <class NodePtr>
    NodePtr rb_tree_next(NodePtr _node)  
    {
        if (_node->right != nullptr)
        {
            return rb_tree_min(_node->right);
        }
            
        while (!rb_tree_is_lchild(_node))
        {
            _node = _node->parent;
        }
            
        return _node->parent;
    }

    //---------------------------�����㷨---------------------------
    /*---------------------------------------*\
    |       p                         p       |
    |      / \                       / \      |
    |     _x   d    rotate left      y   d     |
    |    / \       ===========>    / \        |
    |   a   y                     _x   c       |
    |      / \                   / \          |
    |     b   c                 a   b         |
    \*---------------------------------------*/
    // ����������һΪ�����㣬������Ϊ���ڵ�
    template <class NodePtr>
    void rb_tree_rotate_left(NodePtr _x, NodePtr& _root)  
    {
        auto y = _x->right;  // y Ϊ _x �����ӽڵ�
        _x->right = y->left;
        if (y->left != nullptr)
            y->left->parent = _x;
        y->parent = _x->parent;

        if (_x == _root)
        { // ��� _x Ϊ���ڵ㣬�� y ���� _x ��Ϊ���ڵ�
            _root = y;
        }
        else if (rb_tree_is_lchild(_x))
        { // ��� _x �����ӽڵ�
            _x->parent->left = y;
        }
        else
        { // ��� _x �����ӽڵ�
            _x->parent->right = y;
        }
        // ���� _x �� y �Ĺ�ϵ
        y->left = _x;
        _x->parent = y;
    }

    /*----------------------------------------*\
    |     p                         p          |
    |    / \                       / \         |
    |   d   _x      rotate right   d   y        |
    |      / \     ===========>      / \       |
    |     y   a                     b   _x      |
    |    / \                           / \     |
    |   b   c                         c   a    |
    \*----------------------------------------*/
    // ����������һΪ�����㣬������Ϊ���ڵ�
    template <class NodePtr>
    void rb_tree_rotate_right(NodePtr _x, NodePtr& _root)  
    {
        auto y = _x->left;
        _x->left = y->right;
        if (y->right)
            y->right->parent = _x;
        y->parent = _x->parent;

        if (_x == _root)
        { // ��� _x Ϊ���ڵ㣬�� y ���� _x ��Ϊ���ڵ�
            _root = y;
        }
        else if (rb_tree_is_lchild(_x))
        { // ��� _x �����ӽڵ�
            _x->parent->left = y;
        }
        else
        { // ��� _x �����ӽڵ�
            _x->parent->right = y;
        }
        // ���� _x �� y �Ĺ�ϵ
        y->right = _x;
        _x->parent = y;
    }

    // ����ڵ��ʹ rb tree ����ƽ�⣬����һΪ�����ڵ㣬������Ϊ���ڵ�
    //
    // case 1: �����ڵ�λ�ڸ��ڵ㣬�������ڵ�Ϊ��
    // case 2: �����ڵ�ĸ��ڵ�Ϊ�ڣ�û���ƻ�ƽ�⣬ֱ�ӷ���
    // case 3: ���ڵ������ڵ㶼Ϊ�죬��ڵ������ڵ�Ϊ�ڣ��游�ڵ�Ϊ�죬
    //         Ȼ�����游�ڵ�Ϊ��ǰ�ڵ㣬��������
    // case 4: ���ڵ�Ϊ�죬����ڵ�Ϊ NIL ���ɫ�����ڵ�Ϊ���ң����ӣ���ǰ�ڵ�Ϊ�ң��󣩺��ӣ�
    //         �ø��ڵ��Ϊ��ǰ�ڵ㣬���Ե�ǰ�ڵ�Ϊ֧�����ң���
    // case 5: ���ڵ�Ϊ�죬����ڵ�Ϊ NIL ���ɫ�����ڵ�Ϊ���ң����ӣ���ǰ�ڵ�Ϊ���ң����ӣ�
    //         �ø��ڵ��Ϊ��ɫ���游�ڵ��Ϊ��ɫ�����游�ڵ�Ϊ֧���ң�����
    //
    // �ο�����: http://blog.csdn.net/v_JULY_v/article/details/6105630
    //          http://blog.csdn.net/v_JULY_v/article/details/6109153
    template <class NodePtr>
    void rb_tree_insert_rebalance(NodePtr _x, NodePtr& _root)  
    {
        rb_tree_set_red(_x);  // �����ڵ�Ϊ��ɫ
        while (_x != _root && rb_tree_is_red(_x->parent))
        {
            if (rb_tree_is_lchild(_x->parent))
            { // ������ڵ������ӽڵ�
                auto uncle = _x->parent->parent->right;
                if (uncle != nullptr && rb_tree_is_red(uncle))
                { // case 3: ���ڵ������ڵ㶼Ϊ��
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_black(uncle);
                    _x = _x->parent->parent;
                    rb_tree_set_red(_x);
                }
                else
                { // ������ڵ������ڵ�Ϊ��
                    if (!rb_tree_is_lchild(_x))
                    { // case 4: ��ǰ�ڵ� _x Ϊ���ӽڵ�
                        _x = _x->parent;
                        rb_tree_rotate_left(_x, _root);
                    }
                    // ��ת���� case 5�� ��ǰ�ڵ�Ϊ���ӽڵ�
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_red(_x->parent->parent);
                    rb_tree_rotate_right(_x->parent->parent, _root);
                    break;
                }
            }
            else  // ������ڵ������ӽڵ㣬�Գƴ���
            {
                auto uncle = _x->parent->parent->left;
                if (uncle != nullptr && rb_tree_is_red(uncle))
                { // case 3: ���ڵ������ڵ㶼Ϊ��
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_black(uncle);
                    _x = _x->parent->parent;
                    rb_tree_set_red(_x);
                    // ��ʱ�游�ڵ�Ϊ�죬���ܻ��ƻ�����������ʣ��ǰ�ڵ�Ϊ�游�ڵ㣬��������
                }
                else
                { // ������ڵ������ڵ�Ϊ��
                    if (rb_tree_is_lchild(_x))
                    { // case 4: ��ǰ�ڵ� _x Ϊ���ӽڵ�
                        _x = _x->parent;
                        rb_tree_rotate_right(_x, _root);
                    }
                    // ��ת���� case 5�� ��ǰ�ڵ�Ϊ���ӽڵ�
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_red(_x->parent->parent);
                    rb_tree_rotate_left(_x->parent->parent, _root);
                    break;
                }
            }
        }
        rb_tree_set_black(_root);  // ���ڵ���ԶΪ��
    }

    // ɾ���ڵ��ʹ rb tree ����ƽ�⣬����һΪҪɾ���Ľڵ㣬������Ϊ���ڵ㣬������Ϊ��С�ڵ㣬������Ϊ���ڵ�
    // 
    // �ο�����: http://blog.csdn.net/v_JULY_v/article/details/6105630
    //          http://blog.csdn.net/v_JULY_v/article/details/6109153
    template <class NodePtr>
    NodePtr rb_tree_erase_rebalance(NodePtr z, NodePtr& _root, NodePtr& leftmost, NodePtr& rightmost)
    {
        // y �ǿ��ܵ��滻�ڵ㣬ָ������Ҫɾ���Ľڵ�
        auto y = (z->left == nullptr || z->right == nullptr) ? z : rb_tree_next(z);
        // _x �� y ��һ�����ӽڵ�� NIL �ڵ�
        auto _x = y->left != nullptr ? y->left : y->right;
        // xp Ϊ _x �ĸ��ڵ�
        NodePtr xp = nullptr;

        // y != z ˵�� z �������ǿ��ӽڵ㣬��ʱ y ָ�� z ������������ڵ㣬_x ָ�� y �����ӽڵ㡣
        // �� y ���� z ��λ�ã��� _x ���� y ��λ�ã������ y ָ�� z
        if (y != z)
        {
            z->left->parent = y;
            y->left = z->left;

            // ��� y ���� z �����ӽڵ㣬��ô z �����ӽڵ�һ��������
            if (y != z->right)
            { // _x �滻 y ��λ��
                xp = y->parent;
                if (_x != nullptr)
                    _x->parent = y->parent;

                y->parent->left = _x;
                y->right = z->right;
                z->right->parent = y;
            }
            else
            {
                xp = y;
            }

            // ���� y �� z �ĸ��ڵ� 
            if (_root == z)
                _root = y;
            else if (rb_tree_is_lchild(z))
                z->parent->left = y;
            else
                z->parent->right = y;
            y->parent = z->parent;
            LT::swap(y->color, z->color);
            y = z;
        }
        // y == z ˵�� z ����ֻ��һ������
        else
        {
            xp = y->parent;
            if (_x)
                _x->parent = y->parent;

            // ���� _x �� z �ĸ��ڵ�
            if (_root == z)
                _root = _x;
            else if (rb_tree_is_lchild(z))
                z->parent->left = _x;
            else
                z->parent->right = _x;

            // ��ʱ z �п���������ڵ�����ҽڵ㣬��������
            if (leftmost == z)
                leftmost = _x == nullptr ? xp : rb_tree_min(_x);
            if (rightmost == z)
                rightmost = _x == nullptr ? xp : rb_tree_max(_x);
        }

        // ��ʱ��y ָ��Ҫɾ���Ľڵ㣬_x Ϊ����ڵ㣬�� _x �ڵ㿪ʼ������
        // ���ɾ���Ľڵ�Ϊ��ɫ����������û�б��ƻ����������������������_x Ϊ���ӽڵ�Ϊ������
        // case 1: �ֵܽڵ�Ϊ��ɫ����ڵ�Ϊ�죬�ֵܽڵ�Ϊ�ڣ��������ң�������������
        // case 2: �ֵܽڵ�Ϊ��ɫ���������ӽڵ㶼Ϊ��ɫ�� NIL�����ֵܽڵ�Ϊ�죬���ڵ��Ϊ��ǰ�ڵ㣬��������
        // case 3: �ֵܽڵ�Ϊ��ɫ�����ӽڵ�Ϊ��ɫ�� NIL�����ӽڵ�Ϊ��ɫ�� NIL��
        //         ���ֵܽڵ�Ϊ�죬�ֵܽڵ�����ӽڵ�Ϊ�ڣ����ֵܽڵ�Ϊ֧���ң���������������
        // case 4: �ֵܽڵ�Ϊ��ɫ�����ӽڵ�Ϊ��ɫ�����ֵܽڵ�Ϊ���ڵ����ɫ�����ڵ�Ϊ��ɫ���ֵܽڵ�����ӽڵ�
        //         Ϊ��ɫ���Ը��ڵ�Ϊ֧�����ң������������ʵ�����ɣ��㷨����
        if (!rb_tree_is_red(y))
        { // _x Ϊ��ɫʱ������������ֱ�ӽ� _x ��Ϊ��ɫ����
            while (_x != _root && (_x == nullptr || !rb_tree_is_red(_x)))
            {
                if (_x == xp->left)
                { // ��� _x Ϊ���ӽڵ�
                    auto brother = xp->right;
                    if (rb_tree_is_red(brother))
                    { // case 1
                        rb_tree_set_black(brother);
                        rb_tree_set_red(xp);
                        rb_tree_rotate_left(xp, _root);
                        brother = xp->right;
                    }
                    // case 1 תΪΪ�� case 2��3��4 �е�һ��
                    if ((brother->left == nullptr || !rb_tree_is_red(brother->left)) &&
                        (brother->right == nullptr || !rb_tree_is_red(brother->right)))
                    { // case 2
                        rb_tree_set_red(brother);
                        _x = xp;
                        xp = xp->parent;
                    }
                    else
                    {
                        if (brother->right == nullptr || !rb_tree_is_red(brother->right))
                        { // case 3
                            if (brother->left != nullptr)
                                rb_tree_set_black(brother->left);
                            rb_tree_set_red(brother);
                            rb_tree_rotate_right(brother, _root);
                            brother = xp->right;
                        }
                        // תΪ case 4
                        brother->color = xp->color;
                        rb_tree_set_black(xp);
                        if (brother->right != nullptr)
                            rb_tree_set_black(brother->right);
                        rb_tree_rotate_left(xp, _root);
                        break;
                    }
                }
                else  // _x Ϊ���ӽڵ㣬�Գƴ���
                {
                    auto brother = xp->left;
                    if (rb_tree_is_red(brother))
                    { // case 1
                        rb_tree_set_black(brother);
                        rb_tree_set_red(xp);
                        rb_tree_rotate_right(xp, _root);
                        brother = xp->left;
                    }
                    if ((brother->left == nullptr || !rb_tree_is_red(brother->left)) &&
                        (brother->right == nullptr || !rb_tree_is_red(brother->right)))
                    { // case 2
                        rb_tree_set_red(brother);
                        _x = xp;
                        xp = xp->parent;
                    }
                    else
                    {
                        if (brother->left == nullptr || !rb_tree_is_red(brother->left))
                        { // case 3
                            if (brother->right != nullptr)
                                rb_tree_set_black(brother->right);
                            rb_tree_set_red(brother);
                            rb_tree_rotate_left(brother, _root);
                            brother = xp->left;
                        }
                        // תΪ case 4
                        brother->color = xp->color;
                        rb_tree_set_black(xp);
                        if (brother->left != nullptr)
                            rb_tree_set_black(brother->left);
                        rb_tree_rotate_right(xp, _root);
                        break;
                    }
                }
            }
            if (_x != nullptr)
                rb_tree_set_black(_x);
        }
        return y;
    }

    //------------------------------------------------��������-----------------------------------------------------------------
    template<class T, class Comp, class Alloc = LT::allocator<T>>
    class rb_tree
    {
        //������������
    public:
        typedef rb_tree_traits<T>                        tree_traits;
        typedef rb_tree_value_traits<T>                  value_traits;

        typedef typename tree_traits::base_type          base_type;
        typedef typename tree_traits::base_ptr           base_ptr;
        typedef typename tree_traits::node_type          node_type;
        typedef typename tree_traits::node_ptr           node_ptr;
        typedef typename tree_traits::key_type           key_type;
        typedef typename tree_traits::mapped_type        mapped_type;
        typedef typename tree_traits::value_type         value_type;
        typedef Comp                                     key_compare;
        typedef rb_tree_color_type                       color_type;

        typedef T*                              pointer;
        typedef const value_type*               const_pointer;
        typedef typename T&                     reference;
        typedef typename const T&               const_reference;
        typedef typename size_t                 size_type;
        typedef typename ptrdiff_t              difference_type;

        typedef rb_tree_iterator<T>                      iterator;
        typedef rb_tree_const_iterator<T>                const_iterator;
        typedef LT::reverse_iterator<iterator>        reverse_iterator;
        typedef LT::reverse_iterator<const_iterator>  const_reverse_iterator;


        //��Ա����
    private:
        base_ptr header_;
        size_type nodeCount_;
        key_compare keyComp_;

    private:
        base_ptr& root()const { return header_->parent; }
        base_ptr& most_left()const { return header_->left; }
        base_ptr& most_right()const { return header_->right; }
    //**************************************************************************************************************************************
    //****************************************************�ⲿ�ӿ�********************************************************************
    //**************************************************************************************************************************************
    public:
        //-----------------------------------�������---------------------------------------------------
        rb_tree() { __init(); }

        rb_tree(const rb_tree& _rhs) 
        {
            __init();
            if (_rhs.nodeCount_)
            {
                __copy_rb_tree(_rhs.root(), root(), header_);
            }
            
        }
        rb_tree(rb_tree&& _rhs) 
            :header_(_rhs.header_),
            nodeCount_(_rhs.nodeCount_),
            keyComp_(_rhs.keyComp_)
        {
            _rhs.__init();
        }

        rb_tree& operator=(const rb_tree& _rhs)
        {
            if (_rhs != *this || !_rhs.nodeCount_)
            {
                __init();
                __copy_rb_tree(_rhs.root(), root());
            }
            return *this;
        }
        rb_tree& operator=(rb_tree&& _rhs)
        {
            if (_rhs != *this)
            {
                __init();
                swap(_rhs);
            }
        }

        ~rb_tree() 
        { 
            clear();
            __deallocate_one_node(header_);
        }

    public:
        // ��������ز���

        iterator               begin()          
        {
            return  __leftmost();
        }
        const_iterator         begin()   const  
        {
            return __leftmost();
        }
        iterator               end()            
        {
            return header_;
        }
        const_iterator         end()     const  
        {
            return header_;
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

        // ������ز���
        bool      empty()    const   { return !node_count_; }
        size_type size()     const   { return node_count_; }
        size_type max_size() const   { return static_cast<size_type>(-1); }

        // ����ɾ����ز���

        // emplace
        template <class ...Args>
        iterator  emplace_multi(Args&& ..._args)
        {
            node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
            LT::pair<base_ptr, bool> insertPair = __insert_pos_multi(value_traits::get_key(newNode->value);
            return __insert_node(insertPair.first, newNode, insertPair.second);
        }

        template <class ...Args>
        LT::pair<iterator, bool> emplace_unique(Args&& ...args)
        {
            node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
            LT::pair<LT::pair<base_ptr, bool>, bool> insertPairPair = __insert_pos_unique(value_traits::get_key(newNode->value);
            pair<iterator, bool> ret = make_pair(iterator(insertPairPair.first.first), insertPairPair.second);
            if (insertPairPair.second)
            {
                ret.first = __insert_node(insertPairPair.first.first, newNode, insertPairPair.first.second);
                return ret;
            }
            __destroy_one_node(newNode);
            __deallocate_one_node(newNode);
            return ret;
        }
  
        template <class ...Args>
        iterator  emplace_multi_use_hint(iterator _hint, Args&& ..._args)
        {
            node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
            if (nodeCount_ == 0)
            {
                return __insert_node(header_, newNode, true);
            }
            key_type key = value_traits::get_key(newNode->value);
            if (_hint == begin())
            { // λ�� begin ��
                if (keyComp_(key, value_traits::get_key(*_hint)))
                {
                    return insert_node_at(_hint.node, newNode, true);
                }
                else
                {
                    auto pos = __insert_pos_multi(key);
                    return __insert_node(pos.first, newNode, pos.second);
                }
            }
            else if (_hint == end())
            { // λ�� end ��
                if (!keyComp_(key, value_traits::get_key(most_right()->get_node_ptr()->value)))
                {
                    return __insert_node(most_right(), newNode, false);
                }
                else
                {
                    auto pos = insert_pos_multi(key);
                    return __insert_node(pos.first, newNode, pos.second);
                }
            }
            return insert_multi_use_hint(_hint, key, newNode);

        }
  
        template <class ...Args>
        iterator  emplace_unique_use_hint(iterator _hint, Args&& ..._args)
        {
            node_ptr newNode = __create_one_node(LT::forward<Args>(_args)...);
            if (nodeCount_ == 0)
            {
                return __insert_node(header_, newNode, true);
            }
            key_type key = value_traits::get_key(newNode->value);
            if (_hint == begin())
            { // λ�� begin ��
                if (keyComp_(key, value_traits::get_key(*_hint)))
                {
                    return __insert_node(_hint.node_, newNode, true);
                }
                else
                {
                    auto pos = __get_insert_unique_pos(key);
                    if (!pos.second)
                    {
                        destroy_node(newNode);
                        return pos.first.first;
                    }
                    return __insert_node(pos.first.first, newNode, pos.first.second);
                }
            }
            else if (_hint == end())
            { // λ�� end ��
                if (keyComp_(value_traits::get_key(most_right()->get_node_ptr()->value), key))
                {
                    return __insert_node(most_right(), newNode, false);
                }
                else
                {
                    auto pos = __insert_pos_unique(key);
                    if (!pos.second)
                    {
                        __destroy_one_node(newNode);
                        return pos.first.first;
                    }
                    return __insert_node(pos.first.first, newNode, pos.first.second);
                }
            }
            //��Чhint����ʹ��
            return insert_unique_use_hint(_hint, key, newNode); 
        }
        // insert

        iterator  insert_multi(const value_type& _value)
        {
            LT::pair<base_ptr, bool> insertPair = __insert_pos_multi(value_traits::get_key(_value));
            return __insert_node(insertPair.first, _value, insertPair.second);
        }
        iterator  insert_multi(value_type&& _value)
        {
            return emplace_multi(LT::move(_value));
        }

        iterator  insert_multi(iterator _hint, const value_type& _value)
        {
            return emplace_multi_use_hint(_hint, _value);
        }
        iterator  insert_multi(iterator _hint, value_type&& _value)
        {
            return emplace_multi_use_hint(_hint, LT::move(_value));
        }

        template <class InputIterator>
        void      insert_multi(InputIterator _first, InputIterator _last)
        {
            size_type n = LT::distance(_first, _last);
            for (; n > 0; --n, ++_first)
                insert_multi(end(), *_first);
        }

        LT::pair<iterator, bool> insert_unique(const value_type& _value)
        {
            LT::pair<LT::pair<base_ptr, bool>, bool> insertPairPair = __insert_pos_unique(value_traits::get_key(_value);
            pair<iterator, bool> ret = make_pair(iterator(insertPairPair.first.first), insertPairPair.second);
            if (insertPairPair.second)
            {
                node_ptr newNode = __create_one_node(_value);
                return make_pair(__insert_node(insertPairPair.first.first, newNode, insertPairPair.first.second), true);
            }
            return ret;
        }
        LT::pair<iterator, bool> insert_unique(value_type&& _value)
        {
            return emplace_unique(LT::move(_value));
        }

        iterator  insert_unique(iterator _hint, const value_type& _value)
        {
            return emplace_unique_use_hint(_hint, _value);
        }
        iterator  insert_unique(iterator _hint, value_type&& _value)
        {
            return emplace_unique_use_hint(_hint, LT::move(_value));
        }

        template <class InputIterator>
        void      insert_unique(InputIterator _first, InputIterator _last)
        {
            size_type n = LT::distance(_first, last);
            for (; n > 0; --n, ++_first)
            {
                insert_unique(end(), *_first);
            }    
        }

        // erase

        iterator  erase(iterator _hint)
        {
            node_ptr node = _hint.node->get_node_ptr();
            iterator next(node);
            ++next;

            rb_tree_erase_rebalance(_hint.node, root(), leftmost(), rightmost());
            destroy_node(node);
            --nodeCount;
            return next;
        }

        size_type erase_multi(const key_type& _key)
        {
            erase(lower_bound(_key), upper_bound(_key));
        }
        size_type erase_unique(const key_type& _key)
        {
            iterator nodeToDelete = find(_key);
            if (nodeToDelete == end())
            {
                return 0;
            }
            erase(nodeToDelete);
            return;
        }

        void      erase(iterator _first, iterator _last)
        {
            if (_first == begin() && _last = end())
            {
                clear();
            }
            while (_first != _last)
            {
                erase(_first++);
            }
        }

        void      clear()
        {
            __delete_rb_tree(root());
            __reset_header();
        }

        // rb_tree ��ز���

        iterator       find(const key_type& _key)
        {
            base_ptr grand = header_;
            base_ptr root = root();
            while (root)
            {
                if (!keyComp_(value_traits::get_key(root->get_node_ptr()->get_value()), _key))
                {
                    //�����key�ȵ�ǰ�ڵ�ֵС,���ߵ��ڵ�ǰ�ڵ�
                    grand == root;
                    root = root->left;
                }
                else
                {
                    //����Ǵ��ڵ���
                    root = root->right
                }
            }

            iterator j = iterator(grand);
            //���keyС�ڵ��ڵ�ǰ�ڵ㣬���ߵ�ǰ�ڵ���header�ڵ㣬����end()��
            return (j == end() || keyComp_(_key, value_traits::get_key(*j))) ? end() : j; 
        }
        const_iterator find(const key_type& _key) const
        {
            base_ptr grand = header_;
            base_ptr root = root();
            while (root)
            {
                if (!keyComp_(value_traits::get_key(root->get_node_ptr()->get_value()), _key))
                {
                    //�����key�ȵ�ǰ�ڵ�ֵС,���ߵ��ڵ�ǰ�ڵ�
                    grand == root;
                    root = root->left;
                }
                else
                {
                    //����Ǵ��ڵ���
                    root = root->right
                }
            }

            const_iterator j = const_iterator(grand);
            //���keyС�ڵ��ڵ�ǰ�ڵ㣬���ߵ�ǰ�ڵ���header�ڵ㣬����end()��
            return (j == end() || keyComp_(_key, value_traits::get_key(*j))) ? end() : j;
        }

        size_type      count_multi(const key_type& _key) const
        {
            auto p = equal_range_multi(_key);
            return static_cast<size_type>(LT::distance(p._first, p.second));
        }
        size_type      count_unique(const key_type& _key) const
        {
            return find(_key) != end() ? 1 : 0;
        }

        iterator       lower_bound(const key_type& _key)
        {
            base_ptr grand = header_;
            base_ptr parent = root();
            while (parent != nullptr)
            {
                //ֻҪ��ǰ��_keyС�ڵ���root��ֵ��������
                if (!keyComp_(value_traits::get_key(parent->get_node_ptr()->value), _key))
                { 
                    grand = parent, parent = parent->left;
                }
                else
                {
                    parent = parent->right;
                }
            }
            return iterator(grand);
        }
        const_iterator lower_bound(const key_type& _key) const
        {
            base_ptr grand = header_;
            base_ptr parent = root();
            while (parent != nullptr)
            {
                //ֻҪ��ǰ��_keyС�ڵ���root��ֵ��������
                if (!keyComp_(value_traits::get_key(parent->get_node_ptr()->value), _key))
                {
                    grand = parent, parent = parent->left;
                }
                else
                {
                    parent = parent->right;
                }
            }
            return const_iterator(grand);
        }

        iterator       upper_bound(const key_type& _key)
        {
            base_ptr grand = header_;
            base_ptr parent = root();
            while (parent)
            {
                //ֻҪ��ǰ��parent��ֵ���ڵ���Ҫ���ҵ�ֵ����������
                if (keycomp_(_key, value_traits::get(parent->get_node_ptr()->value)))
                {
                    grand = parent;
                    parent = parent->left;
                }
                else {
                    parent = parent->right;
                }
            }
            return iterator(parent);
        }
        const_iterator upper_bound(const key_type& _key) const
        {
            base_ptr grand = header_;
            base_ptr parent = root();
            while (parent)
            {
                //ֻҪ��ǰ��parent��ֵ���ڵ���Ҫ���ҵ�ֵ����������
                if (keycomp_(_key, value_traits::get(parent->get_node_ptr()->value)))
                {
                    grand = parent;
                    parent = parent->left;
                }
                else {
                    parent = parent->right;
                }
            }
            return const_iterator(parent);
        }

        LT::pair<iterator, iterator>
            equal_range_multi(const key_type& _key)
        {
            return LT::pair<iterator, iterator>(lower_bound(_key), upper_bound(_key));
        }
        LT::pair<const_iterator, const_iterator>
            equal_range_multi(const key_type& _key) const
        {
            return LT::pair<const_iterator, const_iterator>(lower_bound(_key), upper_bound(_key));
        }

        LT::pair<iterator, iterator>
            equal_range_unique(const key_type& _key)
        {
            iterator it = find(_key);
            auto next = it;
            return it == end() ? LT::make_pair(it, it) : LT::make_pair(it, ++next);
        }
        LT::pair<const_iterator, const_iterator>
            equal_range_unique(const key_type& _key) const
        {
            const_iterator it = find(_key);
            auto next = it;
            return it == end() ? LT::make_pair(it, it) : LT::make_pair(it, ++next);
        }

        void swap(rb_tree& _rhs)
        {
            LT::swap(header_, _rhs.header_);
            LT::swap(nodeCount_, _rhs.nodeCount_);
            LT::swap(keyComp_, _rhs.keyComp_);
        }
    public:
        Alloc get_allocator()const
        {
            return Alloc();
        }
    //**************************************************************************************************************************************
    //****************************************************�ڲ�ʵ��********************************************************************
    //**************************************************************************************************************************************
    private:
        // ����������������ȡ�ø��ڵ㣬��С�ڵ�����ڵ�
        base_ptr& __root()      const { return header_->parent; }
        base_ptr& __leftmost()  const { return header_->left; }
        base_ptr& __rightmost() const { return header_->right; }
    
    private:
        //-----------------------------------------�ڴ�������-----------------------------------------------------------------------
        
        typedef LT::allocator<base_type>        base_allocator;
        typedef LT::allocator<node_type>        node_allocator;

         node_ptr __get_one_node_mem()
        {
            //���һ���ڵ��С���ڴ����򣬵��ǲ���ʼ��,�������ʧ�ܣ�������ָ�룬�����жϳ���
            node_ptr newNode = node_allocator::allocate();
            assert(newNode);
            return newNode;
        }
         //�ú���ֻ������T value��
         template<class ...Args>
         inline void __construct_one_node_mem(node_ptr _node, Args&& ..._args)
         {
             LT::construct(LT::address_of(_node->value), LT::forward<Args>(_args)...);
         }

         inline void __set_one_base_mem(base_ptr _node, base_ptr _parent, base_ptr _left, base_ptr _right, rb_tree_color_type _color)
         {
             _node->parent = _parent;
             _node->left = _left;
             _node->right = _right;
             _node->color = _color;
         }
         
         //����ֵ�½�һ���ڵ�
         template<class ...Args>
         node_ptr __create_one_node(Args &&..._value)
         {
             node_ptr newNode = __get_one_node_mem();
             __construct_one_node_mem(newNode,forward<Args>(_value)...);
             __set_one_base_mem(newNode, nullptr, nullptr, nullptr, _rhs->color_);
             return newNode
         }

         //ֻ�������ݡ�������base_ptr�ڵ�ָ��
         node_ptr __copy_one_node(base_ptr _rhs)
         {
             node_ptr newNode = __get_one_node_mem();
             __construct_one_node_mem(newNode, _rhs->get_node_ptr()->value);
             __set_one_base_mem(newNode, nullptr, nullptr, nullptr, _rhs->color_);
             return newNode;
         }

        //�������ڴ����

         //�ú���T value
         void __destroy_one_node(node_ptr _node)
         {
             LT::destroy(LT::address_of(_node->value));
         }

         //
         void __deallocate_one_node(node_ptr _node)
         {
             node_allocator::deallocate(_node);
         }

      
        //-----------------------------------------������ص��ڲ�ʵ��---------------------------------------------------------------
        
        //�޲����½�һ�ſ���     
        inline void __init()
        {
             header_ = __get_one_node_mem();
             __set_one_base_mem(header_, nullptr, header_, header_, red_node);
             nodeCount_ = 0;
        }

        //������nodeΪ�ڵ��һ������_nodeParent�ǿ���֮����½ڵ�ĸ��ڵ㡣
        //�汾һ
        node_ptr __copy_rb_tree(node_ptr _nodeToCopy, node_ptr _nodeParent)
        {
            try
            {
                node_ptr newNode = __copy_one_node(_nodeToCopy);
                newNode->parent = _nodeParent;
                if (_nodeToCopy->left)
                {
                    newNode->left = __copy_rb_tree(_nodeParent->left, newNode);
                }
                if (_nodeToCopy->right)
                {
                    newNode->right = __copy_rb_tree(_nodeParent->right, newNode);
                }
            }
            catch (...)
            {
                __delete_rb_tree(newNode);
            }
        }
        //�汾��
        node_ptr __copy_rb_tree(node_ptr _nodeToCopy, node_ptr _nodeParent, node_ptr _header)
        {
            try
            {
                node_ptr newNode = __copy_one_node(_nodeToCopy);
                newNode->parent = _nodeParent;
                header_->parent = newNode;
                if (_nodeToCopy->left)
                {
                    newNode->left = __copy_rb_tree(_nodeParent->left, newNode);
                }
                if (_nodeToCopy->right)
                {
                    newNode->right = __copy_rb_tree(_nodeParent->right, newNode);
                }
            }
            catch (...)
            {
                __delete_rb_tree(newNode);
                header_->parent = nullptr;
            }
        }
       
        //ɾ����rootΪ���ڵ��һ������_nodeParent�ǿ���֮����½ڵ�ĸ��ڵ㡣
        void __delete_rb_tree(node_ptr _root)
        {
            if (_root)
            {
                __delete_rb_tree(_root->left);
                __delete_rb_tree(_root->right);
                __destroy_one_node(_root);
                __deallocate_one_node(_root);
            }
        }

        //---------------------------------------------------��������ڲ�ʵ��------------------------------------------------

        //����һ��value,ȷ�������λ��,ǰ��������Ϊ��
        //�汾һ
        LT::pair<pair<base_ptr, bool>, bool /*maybe do not need insert*/> __insert_pos_unique(const key_type& _key)
        {
            base_ptr grand = header_;
            base_ptr parent = root();
            bool insertToLeft = true;
            while (parent)
            {
                grand = parent;
                insertToLeft = keyComp_(_key, value_traits::get_key(child->get_node_ptr()->value));
                parent = insertToLeft ? parent->left : parent->right;
            }
            //�ж��Ƿ�����ظ�������ǰ���whileѭ���У�ֻҪroot��ֵ���ڵ���key����������ߡ�
            iterator root = iterator(parent);

            //������������룬��ô��ǰ�ڵ�һ�����ڵ���key
            if (insertToLeft)
            {
                if (header_ == grand || grand == most_left())
                {
                    return make_pair(make_pair(grand, true), true);
                }
                //ͨ��ȡrootǰһ����ֵ�����Ի�֪���������ֵ����key��Ԫ�أ���ôǰһ��ֵһ����keyС
                --root;
            }
            if (keyComp_(value_traits::get_key(*root), key)
            {
                return make_pair(make_pair(grand, insertToleft), true);
            }
            return make_pair(make_pair(grand, insertToleft), false);
        }
        //�汾��
        LT::pair<base_ptr/*pNode to insert*/, bool/*is insert in left*/> __insert_pos_multi(const key_type& _key)
        {
            base_ptr grandParent = header_;
            base_ptr parent = root();
            bool insertToLeft = true;
            while (parent)
            {
                grandParent = parent;
                insertToLeft = keyComp_(_key, value_traits::get_key(parent->get_node_ptr()->value));
                parent = insertToLeft ? parent->left : parent->right;
            }
            return make_pair(grandParent, insertToLeft);
        }


        iterator __insert_value(iterator _root, value_type _value, bool _insertToLeft)
        {
            base_ptr newNode = __create_one_node(_value)->get_base_ptr();
            newNode->parent = _root;
            if (_root == header_)
            {
                root() = newNode;
                most_left() = newNode;
                most_right() = newNode;
            }
            else if (_root == most_left())
            {
                _root->left = newNode;
                if (_root = most_left())
                {
                    most_left() = newNode;
                }
            }
            else
            {
                _root->right = newNode;
                if (_root == most_right())
                {
                    most_right() = newNode;
                }
            }

            rb_tree_insert_rebalance(newNode, root());
            ++nodeCount_;
            return iterator(newNode);
        }

        iterator __insert_node(iterator _root, node_ptr _newNode, bool _insertToLeft)
        {
            base_ptr newNode = _newNode->get_base_ptr();
            newNode->parent = _root;
            if (_root == header_)
            {
                root() = newNode;
                most_left() = newNode;
                most_right() = newNode;
            }
            else if (_root == most_left())
            {
                _root->left = newNode;
                if (_root = most_left())
                {
                    most_left() = newNode;
                }
            }
            else
            {
                _root->right = newNode;
                if (_root == most_right())
                {
                    most_right() = newNode;
                }
            }

            rb_tree_insert_rebalance(newNode, root());
            ++nodeCount_;
            return iterator(_newNode);
        }

        // ����Ԫ�أ���ֵ�����ظ���ʹ�� hint
        template <class T, class Compare>
        typename rb_tree<T, Compare>::iterator
            rb_tree<T, Compare>::
            insert_multi_use_hint(iterator hint, key_type key, node_ptr node)
        {
            // �� hint ����Ѱ�ҿɲ����λ��
            auto np = hint.node;
            auto before = hint;
            --before;
            auto bnp = before.node;
            if (!key_comp_(key, value_traits::get_key(*before)) &&
                !key_comp_(value_traits::get_key(*hint), key))
            { // before <= node <= hint
                if (bnp->right == nullptr)
                {
                    return insert_node_at(bnp, node, false);
                }
                else if (np->left == nullptr)
                {
                    return insert_node_at(np, node, true);
                }
            }
            auto pos = get_insert_multi_pos(key);
            return insert_node_at(pos.first, node, pos.second);
        }

        // ����Ԫ�أ���ֵ�������ظ���ʹ�� hint
        template <class T, class Compare>
        typename rb_tree<T, Compare>::iterator
            rb_tree<T, Compare>::
            insert_unique_use_hint(iterator hint, key_type key, node_ptr node)
        {
            // �� hint ����Ѱ�ҿɲ����λ��
            auto np = hint.node;
            auto before = hint;
            --before;
            auto bnp = before.node;
            if (key_comp_(value_traits::get_key(*before), key) &&
                key_comp_(key, value_traits::get_key(*hint)))
            { // before < node < hint
                if (bnp->right == nullptr)
                {
                    return insert_node_at(bnp, node, false);
                }
                else if (np->left == nullptr)
                {
                    return insert_node_at(np, node, true);
                }
            }
            auto pos = get_insert_unique_pos(key);
            if (!pos.second)
            {
                destroy_node(node);
                return pos.first.first;
            }
            return insert_node_at(pos.first.first, node, pos.first.second);
        }
    //----------------------------------------------------��header���-------------------------------------------------------

    //header���ã�������clear()��
        void __reset_header()
        {
            header_->parent = nullptr;
            header_->left = header_;
            header_->right = header_->right;
            nodeCount_ = 0;
        }
    };

};
