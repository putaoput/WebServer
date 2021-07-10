//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once
#include "iterator.h"
#include "functional.h"
#include "stl_heap.h"


//��ͷ�ļ�����ʵ�ֱ�׼���㷨�е������㷨��Ŀǰ�Ѿ�ʵ�ֵ���
//���ֲ���lower_bound��upper_bound, binary_serch
//sort:��ʡʽ����:��Ͽ��ţ����ţ���������

namespace LT {
	//****************************************���ֲ���*******************************************************
	
	//*******************************************************************************************************
    // lower_bound
    // ��[_first, _last)�в��ҵ�һ����С�� _value ��Ԫ�أ�������ָ�����ĵ���������û���򷵻� _last
    //*****************************************************************************************/
    // __lower_bound �� forward_iterator_tag �汾
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

    // __lower_bound �� random_access_iterator_tag �汾
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

    // �Զ���ȽϺ����İ汾
    // __lower_bound �� forward_iterator_tag �汾
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

    // __lower_bound �� random_access_iterator_tag �汾
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
    // ��[_first, _last)�в��ҵ�һ����С�� _value ��Ԫ�أ�������ָ�����ĵ���������û���򷵻� _last
    //*****************************************************************************************/
    // ubound_dispatch �� forward_iterator_tag �汾
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

    // ubound_dispatch �� random_access_iterator_tag �汾
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

    // ���ذ汾ʹ�ú������� comp ����Ƚϲ���
    // ubound_dispatch �� forward_iterator_tag �汾
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

    // ubound_dispatch �� random_access_iterator_tag �汾
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
    // ���ֲ��ң������[first, last)���е�ͬ�� value ��Ԫ�أ����� true�����򷵻� false
    /*****************************************************************************************/
    template <class ForwardIter, class T>
    bool binary_search(ForwardIter first, ForwardIter last, const T& value)
    {
        auto i = LT::lower_bound(first, last, value);
        return i != last && !(value < *i);
    }

    // ���ذ汾ʹ�ú������� comp ����Ƚϲ���
    template <class ForwardIter, class T, class Compared>
    bool binary_search(ForwardIter first, ForwardIter last, const T& value, Compared comp)
    {
        auto i = LT::lower_bound(first, last, value);
        return i != last && !comp(value, *i);
    }

    /*****************************************************************************************/
    // sort
    // :��ʡʽ����:��Ͽ��ţ����ţ���������
    /*****************************************************************************************/

    //��Ԫ������С�ڵ���16ʱ��ֱ��ʹ�ò������򡣷���ʹ��������ֵ�ָ�Ŀ������򣬵���ʱ��ת��ʹ�ö�����
    //��ʹ�ò��������ԭ����ð�ݣ�ѡ�񣬲��������У�������������õġ�
    //��������ð�����ȶ����򣬲���Ҫ����ռ䣬��ԭ���������ΪO(n),�ΪO(n2),ƽ��ΪO(n2)��
    //ѡ�������Ǵ�δ��������ѡ����С�ķ�����������ĩβ��ʱ�临�Ӷ���ã����ƽ����ΪO(n2)
    //�����ں��Ĵ����ϣ���������ֻҪһ����ֵ(�ƶ�)������ð��������Ҫ������������
    template<class RandomIterator, class Comp>
    void __insert_sort(RandomIterator _first, RandomIterator _last, Comp _cmp) {
        for (auto it = _first + 1; it != _last; ++it) {
            auto tmp = *it;
            auto j = it;
            for (; j != _first;)
            {
                if (_cmp(tmp, *(--j))) // ��֤���������ȶ�
                {
                    break;
                }
            }
            //���Ѿ�δ����Ĵ�����뵽���������֮��
            //Ҫ��[j,it)��Ԫ���ƶ���(j,it],Ȼ��*j = tmp;
            for (auto idx = it; idx != j; --idx)
            {
                *idx = *(idx - 1);
            }
            *j = tmp;
        }
    }

    //�ڿ���
    //������ֵ�ָ�
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

    //��������
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

    //��������
    template<class RandomAcessIterator, class Comp>
    void __heap_sort(RandomAcessIterator _first, RandomAcessIterator _last, Comp _cmp) {
        int len = _last - _first;
        //�Ƚ�����
        LT::__make_heap(_first, _last, _cmp);
        //����
        for (int i = len - 1; i >= 0; --i) {
            iter_swap(_first, _first + i);
            LT::__adjust(_first, 0, i, _cmp);
        }
    }

    //����Ľӿ�
    template<class RandomAcessIterator, class Comp>
    void sort(RandomAcessIterator _first, RandomAcessIterator _last,
        Comp _cmp = LT::less<typename iterator_traits<RandomAcessIterator>::value_type>()) {
        //ե������������������
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


