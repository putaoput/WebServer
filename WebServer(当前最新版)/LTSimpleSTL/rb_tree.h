//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//该头文件负责提供基本数据结构:红黑树。以模板的形式定义
//分别主要有pair萃取，红黑树节点设计，红黑树迭代器设计，红黑树设计几个部分
//该头文件的迭代器，结点，红黑树算法实现均参考开源代码，红黑树类设计框架参考开源代码，但是内部具体实现是独立完成
//保证了大体结构的正确，但是内部实现难免会有考虑不周之处

#include <initializer_list>
#include <cassert>

#include "functional.h"
#include "iterator.h"
#include "memory.h"
#include "type_traits.h"
#include "allocator.h"

namespace LT 
{
	//--------------------------首先要定义红黑树节点颜色-------------------------------------------
	typedef bool rb_tree_color_type;
	static constexpr rb_tree_color_type red_node = false;
	static constexpr rb_tree_color_type black_node = true;


    //---------------------------------------forward declaration----------------------------------

    template <class T> struct rb_tree_node_base;
    template <class T> struct rb_tree_node;

    template <class T> struct rb_tree_iterator;
    template <class T> struct rb_tree_const_iterator;

    //----------------------------------红黑树类型萃取---------------------------------------------------

    //非pair类型
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

    //是pair类型
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

    // --------------------------------rb tree 的节点设计--------------------------------

    template <class T>
    struct rb_tree_node_base
    {
        typedef rb_tree_color_type          color_type;
        typedef rb_tree_node_base<T>*       base_ptr;
        typedef rb_tree_node<T>*            node_ptr;

        base_ptr   parent;  // 父节点
        base_ptr   left;    // 左子节点
        base_ptr   right;   // 右子节点
        color_type color;   // 节点颜色

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

        T value_;  // 节点值

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

    // --------------------------------------rb tree 的迭代器-------------------------------------------------

    template <class T>
    struct rb_tree_iterator_base :public LT::iterator<LT::bidirectional_iterator_tag, T>
    {
        typedef typename rb_tree_traits<T>::base_ptr  base_ptr;

        base_ptr node_;  // 指向节点本身

        rb_tree_iterator_base() :node_(nullptr) {}

        // 使迭代器前进
        void inc()
        {
            if (node_->right != nullptr)
            {
                node_ = rb_tree_min(node_->right);
            }
            else
            {  // 如果没有右子节点
                auto y = node_->parent;
                while (y->right == node_)
                {
                    node_ = y;
                    y = y->parent;
                }
                if (node_->right != y)  // 应对“寻找根节点的下一节点，而根节点没有右子节点”的特殊情况
                    node_ = y;
            }
        }

        // 使迭代器后退
        void dec()
        {
            if (node_->parent->parent == node_ && rb_tree_is_red(node_))
            { // 如果 node_ 为 header
                node_ = node_->right;  // 指向整棵树的 max 节点
            }
            else if (node_->left != nullptr)
            {
                node_ = rb_tree_max(node_->left);
            }
            else
            {  // 非 header 节点，也无左子节点
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

        // 构造函数
        rb_tree_iterator() {}
        rb_tree_iterator(base_ptr _x) { node_ = _x; }
        rb_tree_iterator(node_ptr _x) { node_ = _x; }
        rb_tree_iterator(const iterator& _rhs) { node_ = _rhs.node_; }
        rb_tree_iterator(const const_iterator& _rhs) { node_ = _rhs.node_; }

        // 重载操作符
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

        // 构造函数
        rb_tree_const_iterator() {}
        rb_tree_const_iterator(base_ptr _x) { node_ = _x; }
        rb_tree_const_iterator(node_ptr _x) { node_ = _x; }
        rb_tree_const_iterator(const iterator& _rhs) { node_ = _rhs.node_; }
        rb_tree_const_iterator(const const_iterator& _rhs) { node_ = _rhs.node_; }

        // 重载操作符
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

    //----------------------------------------------------红黑树算法------------------------------------------------------------

    //寻找最小节点值
    template <class NodePtr>
    NodePtr rb_tree_min(NodePtr _x)  
    {
        while (_x->left != nullptr)
        {
            _x = _x->left;
        }
            
        return _x;
    }

    //寻找最大节点值
    template <class NodePtr>
    NodePtr rb_tree_max(NodePtr _x)  
    {
        while (_x->right != nullptr)
        {
            _x = _x->right;
        }
            
        return _x;
    }

    //判断当前节点是不是一个左儿子
    template <class NodePtr>
    bool rb_tree_is_lchild(NodePtr _node)  
    {
        return _node == _node->parent->left;
    }

    //判断当前节点是不是红色
    template <class NodePtr>
    bool rb_tree_is_red(NodePtr _node)  
    {
        return _node->color == red_node;
    }

    //把当前节点设为黑色
    template <class NodePtr>
    void rb_tree_set_black(NodePtr& _node)  
    {
        _node->color = black_node;
    }

    //把当前节点设为红色
    template <class NodePtr>
    void rb_tree_set_red(NodePtr& _node)  
    {
        _node->color = red_node;
    }

    //返回当前节点的下一个节点
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

    //---------------------------调整算法---------------------------
    /*---------------------------------------*\
    |       p                         p       |
    |      / \                       / \      |
    |     _x   d    rotate left      y   d     |
    |    / \       ===========>    / \        |
    |   a   y                     _x   c       |
    |      / \                   / \          |
    |     b   c                 a   b         |
    \*---------------------------------------*/
    // 左旋，参数一为左旋点，参数二为根节点
    template <class NodePtr>
    void rb_tree_rotate_left(NodePtr _x, NodePtr& _root)  
    {
        auto y = _x->right;  // y 为 _x 的右子节点
        _x->right = y->left;
        if (y->left != nullptr)
            y->left->parent = _x;
        y->parent = _x->parent;

        if (_x == _root)
        { // 如果 _x 为根节点，让 y 顶替 _x 成为根节点
            _root = y;
        }
        else if (rb_tree_is_lchild(_x))
        { // 如果 _x 是左子节点
            _x->parent->left = y;
        }
        else
        { // 如果 _x 是右子节点
            _x->parent->right = y;
        }
        // 调整 _x 与 y 的关系
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
    // 右旋，参数一为右旋点，参数二为根节点
    template <class NodePtr>
    void rb_tree_rotate_right(NodePtr _x, NodePtr& _root)  
    {
        auto y = _x->left;
        _x->left = y->right;
        if (y->right)
            y->right->parent = _x;
        y->parent = _x->parent;

        if (_x == _root)
        { // 如果 _x 为根节点，让 y 顶替 _x 成为根节点
            _root = y;
        }
        else if (rb_tree_is_lchild(_x))
        { // 如果 _x 是右子节点
            _x->parent->left = y;
        }
        else
        { // 如果 _x 是左子节点
            _x->parent->right = y;
        }
        // 调整 _x 与 y 的关系
        y->right = _x;
        _x->parent = y;
    }

    // 插入节点后使 rb tree 重新平衡，参数一为新增节点，参数二为根节点
    //
    // case 1: 新增节点位于根节点，令新增节点为黑
    // case 2: 新增节点的父节点为黑，没有破坏平衡，直接返回
    // case 3: 父节点和叔叔节点都为红，令父节点和叔叔节点为黑，祖父节点为红，
    //         然后令祖父节点为当前节点，继续处理
    // case 4: 父节点为红，叔叔节点为 NIL 或黑色，父节点为左（右）孩子，当前节点为右（左）孩子，
    //         让父节点成为当前节点，再以当前节点为支点左（右）旋
    // case 5: 父节点为红，叔叔节点为 NIL 或黑色，父节点为左（右）孩子，当前节点为左（右）孩子，
    //         让父节点变为黑色，祖父节点变为红色，以祖父节点为支点右（左）旋
    //
    // 参考博客: http://blog.csdn.net/v_JULY_v/article/details/6105630
    //          http://blog.csdn.net/v_JULY_v/article/details/6109153
    template <class NodePtr>
    void rb_tree_insert_rebalance(NodePtr _x, NodePtr& _root)  
    {
        rb_tree_set_red(_x);  // 新增节点为红色
        while (_x != _root && rb_tree_is_red(_x->parent))
        {
            if (rb_tree_is_lchild(_x->parent))
            { // 如果父节点是左子节点
                auto uncle = _x->parent->parent->right;
                if (uncle != nullptr && rb_tree_is_red(uncle))
                { // case 3: 父节点和叔叔节点都为红
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_black(uncle);
                    _x = _x->parent->parent;
                    rb_tree_set_red(_x);
                }
                else
                { // 无叔叔节点或叔叔节点为黑
                    if (!rb_tree_is_lchild(_x))
                    { // case 4: 当前节点 _x 为右子节点
                        _x = _x->parent;
                        rb_tree_rotate_left(_x, _root);
                    }
                    // 都转换成 case 5： 当前节点为左子节点
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_red(_x->parent->parent);
                    rb_tree_rotate_right(_x->parent->parent, _root);
                    break;
                }
            }
            else  // 如果父节点是右子节点，对称处理
            {
                auto uncle = _x->parent->parent->left;
                if (uncle != nullptr && rb_tree_is_red(uncle))
                { // case 3: 父节点和叔叔节点都为红
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_black(uncle);
                    _x = _x->parent->parent;
                    rb_tree_set_red(_x);
                    // 此时祖父节点为红，可能会破坏红黑树的性质，令当前节点为祖父节点，继续处理
                }
                else
                { // 无叔叔节点或叔叔节点为黑
                    if (rb_tree_is_lchild(_x))
                    { // case 4: 当前节点 _x 为左子节点
                        _x = _x->parent;
                        rb_tree_rotate_right(_x, _root);
                    }
                    // 都转换成 case 5： 当前节点为左子节点
                    rb_tree_set_black(_x->parent);
                    rb_tree_set_red(_x->parent->parent);
                    rb_tree_rotate_left(_x->parent->parent, _root);
                    break;
                }
            }
        }
        rb_tree_set_black(_root);  // 根节点永远为黑
    }

    // 删除节点后使 rb tree 重新平衡，参数一为要删除的节点，参数二为根节点，参数三为最小节点，参数四为最大节点
    // 
    // 参考博客: http://blog.csdn.net/v_JULY_v/article/details/6105630
    //          http://blog.csdn.net/v_JULY_v/article/details/6109153
    template <class NodePtr>
    NodePtr rb_tree_erase_rebalance(NodePtr z, NodePtr& _root, NodePtr& leftmost, NodePtr& rightmost)
    {
        // y 是可能的替换节点，指向最终要删除的节点
        auto y = (z->left == nullptr || z->right == nullptr) ? z : rb_tree_next(z);
        // _x 是 y 的一个独子节点或 NIL 节点
        auto _x = y->left != nullptr ? y->left : y->right;
        // xp 为 _x 的父节点
        NodePtr xp = nullptr;

        // y != z 说明 z 有两个非空子节点，此时 y 指向 z 右子树的最左节点，_x 指向 y 的右子节点。
        // 用 y 顶替 z 的位置，用 _x 顶替 y 的位置，最后用 y 指向 z
        if (y != z)
        {
            z->left->parent = y;
            y->left = z->left;

            // 如果 y 不是 z 的右子节点，那么 z 的右子节点一定有左孩子
            if (y != z->right)
            { // _x 替换 y 的位置
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

            // 连接 y 与 z 的父节点 
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
        // y == z 说明 z 至多只有一个孩子
        else
        {
            xp = y->parent;
            if (_x)
                _x->parent = y->parent;

            // 连接 _x 与 z 的父节点
            if (_root == z)
                _root = _x;
            else if (rb_tree_is_lchild(z))
                z->parent->left = _x;
            else
                z->parent->right = _x;

            // 此时 z 有可能是最左节点或最右节点，更新数据
            if (leftmost == z)
                leftmost = _x == nullptr ? xp : rb_tree_min(_x);
            if (rightmost == z)
                rightmost = _x == nullptr ? xp : rb_tree_max(_x);
        }

        // 此时，y 指向要删除的节点，_x 为替代节点，从 _x 节点开始调整。
        // 如果删除的节点为红色，树的性质没有被破坏，否则按照以下情况调整（_x 为左子节点为例）：
        // case 1: 兄弟节点为红色，令父节点为红，兄弟节点为黑，进行左（右）旋，继续处理
        // case 2: 兄弟节点为黑色，且两个子节点都为黑色或 NIL，令兄弟节点为红，父节点成为当前节点，继续处理
        // case 3: 兄弟节点为黑色，左子节点为红色或 NIL，右子节点为黑色或 NIL，
        //         令兄弟节点为红，兄弟节点的左子节点为黑，以兄弟节点为支点右（左）旋，继续处理
        // case 4: 兄弟节点为黑色，右子节点为红色，令兄弟节点为父节点的颜色，父节点为黑色，兄弟节点的右子节点
        //         为黑色，以父节点为支点左（右）旋，树的性质调整完成，算法结束
        if (!rb_tree_is_red(y))
        { // _x 为黑色时，调整，否则直接将 _x 变为黑色即可
            while (_x != _root && (_x == nullptr || !rb_tree_is_red(_x)))
            {
                if (_x == xp->left)
                { // 如果 _x 为左子节点
                    auto brother = xp->right;
                    if (rb_tree_is_red(brother))
                    { // case 1
                        rb_tree_set_black(brother);
                        rb_tree_set_red(xp);
                        rb_tree_rotate_left(xp, _root);
                        brother = xp->right;
                    }
                    // case 1 转为为了 case 2、3、4 中的一种
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
                        // 转为 case 4
                        brother->color = xp->color;
                        rb_tree_set_black(xp);
                        if (brother->right != nullptr)
                            rb_tree_set_black(brother->right);
                        rb_tree_rotate_left(xp, _root);
                        break;
                    }
                }
                else  // _x 为右子节点，对称处理
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
                        // 转为 case 4
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

    //------------------------------------------------红黑树设计-----------------------------------------------------------------
    template<class T, class Comp, class Alloc = LT::allocator<T>>
    class rb_tree
    {
        //照例定义类型
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


        //成员变量
    private:
        base_ptr header_;
        size_type nodeCount_;
        key_compare keyComp_;

    private:
        base_ptr& root()const { return header_->parent; }
        base_ptr& most_left()const { return header_->left; }
        base_ptr& most_right()const { return header_->right; }
    //**************************************************************************************************************************************
    //****************************************************外部接口********************************************************************
    //**************************************************************************************************************************************
    public:
        //-----------------------------------构造相关---------------------------------------------------
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
        // 迭代器相关操作

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

        // 容量相关操作
        bool      empty()    const   { return !node_count_; }
        size_type size()     const   { return node_count_; }
        size_type max_size() const   { return static_cast<size_type>(-1); }

        // 插入删除相关操作

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
            { // 位于 begin 处
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
            { // 位于 end 处
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
            { // 位于 begin 处
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
            { // 位于 end 处
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
            //无效hint，不使用
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

        // rb_tree 相关操作

        iterator       find(const key_type& _key)
        {
            base_ptr grand = header_;
            base_ptr root = root();
            while (root)
            {
                if (!keyComp_(value_traits::get_key(root->get_node_ptr()->get_value()), _key))
                {
                    //如果是key比当前节点值小,或者等于当前节点
                    grand == root;
                    root = root->left;
                }
                else
                {
                    //如果是大于等于
                    root = root->right
                }
            }

            iterator j = iterator(grand);
            //如果key小于等于当前节点，或者当前节点是header节点，返回end()。
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
                    //如果是key比当前节点值小,或者等于当前节点
                    grand == root;
                    root = root->left;
                }
                else
                {
                    //如果是大于等于
                    root = root->right
                }
            }

            const_iterator j = const_iterator(grand);
            //如果key小于等于当前节点，或者当前节点是header节点，返回end()。
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
                //只要当前的_key小于等于root的值，就往左。
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
                //只要当前的_key小于等于root的值，就往左。
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
                //只要当前的parent的值大于等于要查找的值，就往右走
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
                //只要当前的parent的值大于等于要查找的值，就往右走
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
    //****************************************************内部实现********************************************************************
    //**************************************************************************************************************************************
    private:
        // 以下三个函数用于取得根节点，最小节点和最大节点
        base_ptr& __root()      const { return header_->parent; }
        base_ptr& __leftmost()  const { return header_->left; }
        base_ptr& __rightmost() const { return header_->right; }
    
    private:
        //-----------------------------------------内存分配相关-----------------------------------------------------------------------
        
        typedef LT::allocator<base_type>        base_allocator;
        typedef LT::allocator<node_type>        node_allocator;

         node_ptr __get_one_node_mem()
        {
            //获得一个节点大小的内存区域，但是不初始化,如果分配失败，遇到空指针，将会中断程序
            node_ptr newNode = node_allocator::allocate();
            assert(newNode);
            return newNode;
        }
         //该函数只负责构造T value。
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
         
         //根据值新建一个节点
         template<class ...Args>
         node_ptr __create_one_node(Args &&..._value)
         {
             node_ptr newNode = __get_one_node_mem();
             __construct_one_node_mem(newNode,forward<Args>(_value)...);
             __set_one_base_mem(newNode, nullptr, nullptr, nullptr, _rhs->color_);
             return newNode
         }

         //只拷贝内容。不拷贝base_ptr内的指针
         node_ptr __copy_one_node(base_ptr _rhs)
         {
             node_ptr newNode = __get_one_node_mem();
             __construct_one_node_mem(newNode, _rhs->get_node_ptr()->value);
             __set_one_base_mem(newNode, nullptr, nullptr, nullptr, _rhs->color_);
             return newNode;
         }

        //析构与内存回收

         //该函数T value
         void __destroy_one_node(node_ptr _node)
         {
             LT::destroy(LT::address_of(_node->value));
         }

         //
         void __deallocate_one_node(node_ptr _node)
         {
             node_allocator::deallocate(_node);
         }

      
        //-----------------------------------------构造相关的内部实现---------------------------------------------------------------
        
        //无参数新建一颗空树     
        inline void __init()
        {
             header_ = __get_one_node_mem();
             __set_one_base_mem(header_, nullptr, header_, header_, red_node);
             nodeCount_ = 0;
        }

        //拷贝以node为节点的一棵树，_nodeParent是拷贝之后的新节点的父节点。
        //版本一
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
        //版本二
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
       
        //删除以root为根节点的一棵树，_nodeParent是拷贝之后的新节点的父节点。
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

        //---------------------------------------------------插入相关内部实现------------------------------------------------

        //给定一个value,确定插入的位置,前提是树不为空
        //版本一
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
            //判断是否存在重复。由于前面的while循环中，只要root的值大于等于key，就往左边走。
            iterator root = iterator(parent);

            //如果是往左侧插入，那么当前节点一定大于等与key
            if (insertToLeft)
            {
                if (header_ == grand || grand == most_left())
                {
                    return make_pair(make_pair(grand, true), true);
                }
                //通过取root前一个的值，可以获知如果不存在值等于key的元素，那么前一个值一定比key小
                --root;
            }
            if (keyComp_(value_traits::get_key(*root), key)
            {
                return make_pair(make_pair(grand, insertToleft), true);
            }
            return make_pair(make_pair(grand, insertToleft), false);
        }
        //版本二
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

        // 插入元素，键值允许重复，使用 hint
        template <class T, class Compare>
        typename rb_tree<T, Compare>::iterator
            rb_tree<T, Compare>::
            insert_multi_use_hint(iterator hint, key_type key, node_ptr node)
        {
            // 在 hint 附近寻找可插入的位置
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

        // 插入元素，键值不允许重复，使用 hint
        template <class T, class Compare>
        typename rb_tree<T, Compare>::iterator
            rb_tree<T, Compare>::
            insert_unique_use_hint(iterator hint, key_type key, node_ptr node)
        {
            // 在 hint 附近寻找可插入的位置
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
    //----------------------------------------------------与header相关-------------------------------------------------------

    //header重置，可用于clear()。
        void __reset_header()
        {
            header_->parent = nullptr;
            header_->left = header_;
            header_->right = header_->right;
            nodeCount_ = 0;
        }
    };

};
