//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once
#include "iterator.h"
#include "functional.h"
#include "stl_heap.h"


//该头文件负责实现标准库算法中的其他算法，目前已经实现的有
//二分查找lower_bound，upper_bound, binary_serch
//sort:内省式排序:结合快排，堆排，插入排序

namespace LT {
	//****************************************二分查找*******************************************************
	
	//*******************************************************************************************************
    // lower_bound
    // 在[_first, _last)中查找第一个不小于 _value 的元素，并返回指向它的迭代器，若没有则返回 _last
    //*****************************************************************************************/
    // __lower_bound 的 forward_iterator_tag 版本
    template <class ForwardIter, class T>
    ForwardIter __lower_bound(ForwardIter _first, ForwardIter _last, const T& _value, forward_iterator_tag)
    {
        auto len = LT::distance(_first, _last);
        auto half = len;
        ForwardIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = _first;
            LT::advance(middle, half);
            if (*middle < _value)
            {
                _first = middle;
                ++_first;
                len = len - half - 1;
            }
            else
            {
                len = half;
            }
        }
        return _first;
    }

    // __lower_bound 的 random_access_iterator_tag 版本
    template <class RandomIter, class T>
    RandomIter __lower_bound(RandomIter _first, RandomIter _last,const T& _value, random_access_iterator_tag)
    {
        auto len = _last - _first;
        auto half = len;
        RandomIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = _first + half;
            if (*middle < _value)
            {
                _first = middle + 1;
                len = len - half - 1;
            }
            else
            {
                len = half;
            }
        }
        return _first;
    }

    template <class ForwardIter, class T>
    ForwardIter lower_bound(ForwardIter _first, ForwardIter _last, const T& _value)
    {
        return __lower_bound(_first, _last, _value, iterator_category(_first));
    }

    // 自定义比较函数的版本
    // __lower_bound 的 forward_iterator_tag 版本
    template <class ForwardIter, class T, class Compared>
    ForwardIter  __lower_bound(ForwardIter _first, ForwardIter _last, const T& _value, forward_iterator_tag, Compared comp)
    {
        auto len = distance(_first, _last);
        auto half = len;
        ForwardIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = _first;
            advance(middle, half);
            if (comp(*middle, _value))
            {
                _first = middle;
                ++_first;
                len = len - half - 1;
            }
            else
            {
                len = half;
            }
        }
        return _first;
    }

    // __lower_bound 的 random_access_iterator_tag 版本
    template <class RandomIter, class T, class Compared>
    RandomIter __lower_bound(RandomIter _first, RandomIter _last,const T& _value, random_access_iterator_tag, Compared comp)
    {
        auto len = _last - _first;
        auto half = len;
        RandomIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = _first + half;
            if (comp(*middle, _value))
            {
                _first = middle + 1;
                len = len - half - 1;
            }
            else
            {
                len = half;
            }
        }
        return _first;
    }

    template <class ForwardIter, class T, class Compared>
    ForwardIter lower_bound(ForwardIter _first, ForwardIter _last, const T& _value, Compared comp)
    {
        return __lower_bound(_first, _last, _value, iterator_category(_first), comp);
    }

    //*******************************************************************************************************
    // lower_bound
    // 在[_first, _last)中查找第一个不小于 _value 的元素，并返回指向它的迭代器，若没有则返回 _last
    //*****************************************************************************************/
    // ubound_dispatch 的 forward_iterator_tag 版本
    template <class ForwardIter, class T>
    ForwardIter
        ubound_dispatch(ForwardIter first, ForwardIter last,
            const T& value, forward_iterator_tag)
    {
        auto len = LT::distance(first, last);
        auto half = len;
        ForwardIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = first;
            LT::advance(middle, half);
            if (value < *middle)
            {
                len = half;
            }
            else
            {
                first = middle;
                ++first;
                len = len - half - 1;
            }
        }
        return first;
    }

    // ubound_dispatch 的 random_access_iterator_tag 版本
    template <class RandomIter, class T>
    RandomIter
        ubound_dispatch(RandomIter first, RandomIter last,
            const T& value, random_access_iterator_tag)
    {
        auto len = last - first;
        auto half = len;
        RandomIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = first + half;
            if (value < *middle)
            {
                len = half;
            }
            else
            {
                first = middle + 1;
                len = len - half - 1;
            }
        }
        return first;
    }

    template <class ForwardIter, class T>
    ForwardIter
        upper_bound(ForwardIter first, ForwardIter last, const T& value)
    {
        return LT::ubound_dispatch(first, last, value, iterator_category(first));
    }

    // 重载版本使用函数对象 comp 代替比较操作
    // ubound_dispatch 的 forward_iterator_tag 版本
    template <class ForwardIter, class T, class Compared>
    ForwardIter
        ubound_dispatch(ForwardIter first, ForwardIter last,
            const T& value, forward_iterator_tag, Compared comp)
    {
        auto len = LT::distance(first, last);
        auto half = len;
        ForwardIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = first;
            LT::advance(middle, half);
            if (comp(value, *middle))
            {
                len = half;
            }
            else
            {
                first = middle;
                ++first;
                len = len - half - 1;
            }
        }
        return first;
    }

    // ubound_dispatch 的 random_access_iterator_tag 版本
    template <class RandomIter, class T, class Compared>
    RandomIter
        ubound_dispatch(RandomIter first, RandomIter last,
            const T& value, random_access_iterator_tag, Compared comp)
    {
        auto len = last - first;
        auto half = len;
        RandomIter middle;
        while (len > 0)
        {
            half = len >> 1;
            middle = first + half;
            if (comp(value, *middle))
            {
                len = half;
            }
            else
            {
                first = middle + 1;
                len = len - half - 1;
            }
        }
        return first;
    }

    template <class ForwardIter, class T, class Compared>
    ForwardIter
        upper_bound(ForwardIter first, ForwardIter last, const T& value, Compared comp)
    {
        return LT::ubound_dispatch(first, last, value, iterator_category(first), comp);
    }

    /*****************************************************************************************/
    // binary_search
    // 二分查找，如果在[first, last)内有等同于 value 的元素，返回 true，否则返回 false
    /*****************************************************************************************/
    template <class ForwardIter, class T>
    bool binary_search(ForwardIter first, ForwardIter last, const T& value)
    {
        auto i = LT::lower_bound(first, last, value);
        return i != last && !(value < *i);
    }

    // 重载版本使用函数对象 comp 代替比较操作
    template <class ForwardIter, class T, class Compared>
    bool binary_search(ForwardIter first, ForwardIter last, const T& value, Compared comp)
    {
        auto i = LT::lower_bound(first, last, value);
        return i != last && !comp(value, *i);
    }

    /*****************************************************************************************/
    // sort
    // :内省式排序:结合快排，堆排，插入排序
    /*****************************************************************************************/

    //当元素数量小于等于16时，直接使用插入排序。否则使用三数中值分割的快速排序，当恶化时。转而使用堆排序
    //①使用插入排序的原因：在冒泡，选择，插入排序中，插入排序是最好的。
    //插入排序，冒泡是稳定排序，不需要额外空间，是原地排序最好为O(n),最坏为O(n2),平均为O(n2)。
    //选择排序是从未排序区间选择最小的放入排序区间末尾，时间复杂度最好，最坏，平均均为O(n2)
    //但是在核心代码上，插入排序只要一个赋值(移动)，但是冒泡排序需要三个（交换）
    template<class RandomIterator, class Comp>
    void __insert_sort(RandomIterator _first, RandomIterator _last, Comp _cmp) {
        for (auto it = _first + 1; it != _last; ++it) {
            auto tmp = *it;
            auto j = it;
            for (; j != _first;)
            {
                if (_cmp(tmp, *(--j))) // 保证插入排序稳定
                {
                    break;
                }
            }
            //把已经未排序的代码插入到已排序代码之中
            //要吧[j,it)的元素移动到(j,it],然后*j = tmp;
            for (auto idx = it; idx != j; --idx)
            {
                *idx = *(idx - 1);
            }
            *j = tmp;
        }
    }

    //②快排
    //三数中值分割
    template<class RandomAcessIterator>
    RandomAcessIterator __mid3(RandomAcessIterator _first, RandomAcessIterator _last) {
        auto mid = (_last - _first) / 2 + _first;
        auto last = _last - 1;
        if (*_first > * mid) {
            LT::iter_swap(_first, mid);
        }
        if (*mid > * _last) {
            LT::iter_swap(_last, mid);
        }

        if (*mid < *_first) {
            LT::iter_swap(_first, mid);
        }

        return mid;
    }

    //快排例程
    template<class RandomAcessIterator, class Comp>
    void __quick_sort(RandomAcessIterator _first, RandomAcessIterator _last, Comp _cmp, int _n, int _max) {
        int len = _last - _first;
        if (len <= 1) { return; }
        if (len <= 16) { return __insert_sort(_first, _last, _cmp); }
        if (_n >= _max) { return __heap_sort(_first, _last, _cmp); }

        auto left = _first;
        auto right = _last - 1;
        auto pivot = __mid3(_first, _last);
        while (true) {
            while (++left < right && _cmp(*left, *pivot)) {}
            while (--right < left && _cmp(*pivot, *right)) {}
            if (right - left >= 1) {
                iter_swap(left, right);
            }
            else {
                break;
            }
        }
        __quick_sort(_first, left, _cmp, _n + 1, _max);
        __quick_sort(left + 1, _last, _cmp, _n + 1, _max);
    }

    //堆排例程
    template<class RandomAcessIterator, class Comp>
    void __heap_sort(RandomAcessIterator _first, RandomAcessIterator _last, Comp _cmp) {
        int len = _last - _first;
        //先建立堆
        LT::__make_heap(_first, _last, _cmp);
        //排序
        for (int i = len - 1; i >= 0; --i) {
            iter_swap(_first, _first + i);
            LT::__adjust(_first, 0, i, _cmp);
        }
    }

    //排序的接口
    template<class RandomAcessIterator, class Comp>
    void sort(RandomAcessIterator _first, RandomAcessIterator _last,
        Comp _cmp = LT::less<typename iterator_traits<RandomAcessIterator>::value_type>()) {
        //榨出迭代器的数据类型
        int n = _last - _first;
        if (n > 1 && n <= 16) {
            return __insert_sort(_first, _last, _cmp);
        }
        if (n > 16) {
            int _max = log(n) / log(2);
            return __quick_sort(_first, _last, _cmp, 0, _max);
        }
    }

    template<class RandomAccessIterator>
    void sort(RandomAccessIterator _first, RandomAccessIterator _last) {
        LT::sort(_first, _last, less<typename iterator_traits<RandomAccessIterator>::value_type>());
    }
}


