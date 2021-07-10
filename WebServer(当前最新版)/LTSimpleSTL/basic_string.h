//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once

//写完没想到，原来string的代码量这么大！

//该头文件提供STL实现的字符串类的基础容器,扩容策略与MSVC提供的策略相同
//必须介绍一下该string的扩容策略:初始容量是15(其实占据的内存是16字节，最后一个字节用于保存'\0')。然后每当容量不够时，采用1.5倍扩容
//如果是一次性赋值，那么将会取一次性赋值的大小，但是该大小至少大于15
//当前容量为m，size 为s，如果一次性 += 一个长度为n的字符串，并且m < s + n。那么此时容量将一直按1.5倍扩展，直到capacity() >= s + n;

//自己构思了一种扩容策略，当增加大小为较小的字符串时，1.5倍扩容，否则则扩容到s + n + k。k为常数项，
//比如是借鉴java的扩容常数2。
//不同的扩容策略实现起来很容易，只需要重写/改写函数__expand_capacity()即可，所有的扩容均调用这一函数。//我认为我这样的设置很科学合理
//所以由于暂时没法确定自己的策略更好，就按标准库实现了

//配置内存时用到的技巧是多申请一个字节的容量，每次改变size时，把size后一位设置成'\0'
//原则:改变size_的函数负责set_tail_zero, 改变字符串最后一个字符所在位置的函数负责改变size_
//另外自己写的时候发现， 我很喜欢边写代码，边在代码备注的地方写自己当时的想法
//哈哈哈
#include <assert.h>
#include <iostream>
#include "allocator.h"
#include "alloc.h"
#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "algorithm.h"
#include "util.h"


namespace LT {

    //char 8bit； wchar_t 16bit或者32bit; char16_t 16bit, char32_t 32bit;
    template <class CharType>
    struct char_traits
    {
        typedef CharType char_type;

        static size_t length(const char_type* _str)
        {
            size_t len = 0;
            for (; *_str != char_type(0); ++_str)
                ++len;
            return len;
        }

        static int compare(const char_type* _s1, const char_type* _s2, size_t _n)
        {
            for (; _n != 0; --_n, ++_s1, ++_s2)
            {
                if (*_s1 < *_s2)
                    return -1;
                if (*_s2 < *_s1)
                    return 1;
            }
            return 0;
        }

        static char_type* copy(char_type* _dst, const char_type* _src, size_t _n)
        {
           assert(_src + _n <= _dst || _dst + _n <= _src);//检查区域是否重合
            char_type* r = _dst;
            for (; _n != 0; --_n, ++_dst, ++_src)
                *_dst = *_src;
            return r;
        }

        static char_type* move(char_type* _dst, const char_type* _src, size_t _n)
        {
            char_type* r = _dst;
            if (_dst < _src)
            {
                for (; _n != 0; --_n, ++_dst, ++_src)
                    *_dst = *_src;
            }
            else if (_src < _dst)
            {
                _dst += _n;
                _src += _n;
                for (; _n != 0; --_n)
                    *--_dst = *--_src;
            }
            return r;
        }

        static char_type* fill(char_type* _dst, char_type _ch, size_t _count)
        {
            char_type* r = _dst;
            for (; _count > 0; --_count, ++_dst)
                *_dst = _ch;
            return r;
        }
    };

    template <>
    struct char_traits<char>
    {
        typedef char char_type;

        static size_t length(const char_type* _str)  
        {
            return std::strlen(_str);
        }

        static int compare(const char_type* _s1, const char_type* _s2, size_t _n)  
        {
            return std::memcmp(_s1, _s2, _n);
        }

        static char_type* copy(char_type* _dst, const char_type* _src, size_t _n)  
        {
            assert(_src + _n <= _dst || _dst + _n <= _src);
            return static_cast<char_type*>(std::memcpy(_dst, _src, _n));
        }

        static char_type* move(char_type* _dst, const char_type* _src, size_t _n)  
        {
            return static_cast<char_type*>(std::memmove(_dst, _src, _n));
        }

        static char_type* fill(char_type* _dst, char_type _ch, size_t _count)  
        {
            return static_cast<char_type*>(std::memset(_dst, _ch, _count));
        }
    };

    // Partialized. char_traits<wchar_t>
    template <>
    struct char_traits<wchar_t>
    {
        typedef wchar_t char_type;

        static size_t length(const char_type* _str)  
        {
            return std::wcslen(_str);
        }

        static int compare(const char_type* _s1, const char_type* _s2, size_t _n)  
        {
            return std::wmemcmp(_s1, _s2, _n);
        }

        static char_type* copy(char_type* _dst, const char_type* _src, size_t _n)  
        {
            assert(_src + _n <= _dst || _dst + _n <= _src);
            return static_cast<char_type*>(std::wmemcpy(_dst, _src, _n));
        }

        static char_type* move(char_type* _dst, const char_type* _src, size_t _n)  
        {
            return static_cast<char_type*>(std::wmemmove(_dst, _src, _n));
        }

        static char_type* fill(char_type* _dst, char_type _ch, size_t _count)  
        {
            return static_cast<char_type*>(std::wmemset(_dst, _ch, _count));
        }
    };

    template <>
    struct char_traits<char16_t>
    {
        typedef char16_t char_type;

        static size_t length(const char_type* _str)  
        {
            size_t len = 0;
            for (; *_str != char_type(0); ++_str)
                ++len;
            return len;
        }

        static int compare(const char_type* _s1, const char_type* _s2, size_t _n)  
        {
            for (; _n != 0; --_n, ++_s1, ++_s2)
            {
                if (*_s1 < *_s2)
                    return -1;
                if (*_s2 < *_s1)
                    return 1;
            }
            return 0;
        }

        static char_type* copy(char_type* _dst, const char_type* _src, size_t _n)  
        {
            assert(_src + _n <= _dst || _dst + _n <= _src);
            char_type* r = _dst;
            for (; _n != 0; --_n, ++_dst, ++_src)
                *_dst = *_src;
            return r;
        }

        static char_type* move(char_type* _dst, const char_type* _src, size_t _n)  
        {
            char_type* r = _dst;
            if (_dst < _src)
            {
                for (; _n != 0; --_n, ++_dst, ++_src)
                    *_dst = *_src;
            }
            else if (_src < _dst)
            {
                _dst += _n;
                _src += _n;
                for (; _n != 0; --_n)
                    *--_dst = *--_src;
            }
            return r;
        }

        static char_type* fill(char_type* _dst, char_type _ch, size_t _count)  
        {
            char_type* r = _dst;
            for (; _count > 0; --_count, ++_dst)
                *_dst = _ch;
            return r;
        }
    };
 
    template <>
    struct char_traits<char32_t>
    {
        typedef char32_t char_type;

        static size_t length(const char_type* _str)  
        {
            size_t len = 0;
            for (; *_str != char_type(0); ++_str)
                ++len;
            return len;
        }

        static int compare(const char_type* _s1, const char_type* _s2, size_t _n)  
        {
            for (; _n != 0; --_n, ++_s1, ++_s2)
            {
                if (*_s1 < *_s2)
                    return -1;
                if (*_s2 < *_s1)
                    return 1;
            }
            return 0;
        }

        static char_type* copy(char_type* _dst, const char_type* _src, size_t _n)  
        {
            assert(_src + _n <= _dst || _dst + _n <= _src);
            char_type* r = _dst;
            for (; _n != 0; --_n, ++_dst, ++_src)
                *_dst = *_src;
            return r;
        }

        static char_type* move(char_type* _dst, const char_type* _src, size_t _n)  
        {
            char_type* r = _dst;
            if (_dst < _src)
            {
                for (; _n != 0; --_n, ++_dst, ++_src)
                    *_dst = *_src;
            }
            else if (_src < _dst)
            {
                _dst += _n;
                _src += _n;
                for (; _n != 0; --_n)
                    *--_dst = *--_src;
            }
            return r;
        }

        static char_type* fill(char_type* _dst, char_type _ch, size_t _count)  
        {
            char_type* r = _dst;
            for (; _count > 0; --_count, ++_dst)
                *_dst = _ch;
            return r;
        }
    };


	template<class T, class Traits = char_traits<T>, class Alloc = allocator<T>>
	class basic_string {
		
		//照例进行一些类型定义
	public:
		typedef T											value_type;
		typedef value_type*									pointer;
		typedef const value_type*							const_pointer;
		typedef value_type*									iterator;
		typedef const value_type*							const_iterator;
		typedef LT::reverse_iterator<iterator>				reverse_iterator;
		typedef LT::reverse_iterator<const_iterator>		const_reverse_iterator;
		typedef value_type&									reference;
		typedef const value_type&							const_reference;
		typedef size_t										size_type;
		typedef ptrdiff_t									difference_type;
        typedef Traits                                      char_traits;

		//定义string内部的三个重要的指针
	private:
		iterator  strPtr_;
        size_type size_;//为了保持和c兼容，每次改变size_.都要执行__set_tail_zero(size_);
        size_type capacity_;
    public:
        static constexpr size_type npos = static_cast<size_type>(-1);
	/////********************************************************************************************************
	//**************************************************对外接口**************************************************
	////**********************************************************************************************************
    public:
		//-------------------------------------------构造相关-------------------------------------------------------
		basic_string() 
            :strPtr_(nullptr), size_(0), capacity_(15)
        { 
            __init();
        }
        basic_string(size_type _n, value_type _value) 
            :strPtr_(nullptr), size_(0), capacity_(0)
        { 
            __init_value(_n, _value); 
        }
        basic_string(const basic_string& _str, size_type _pos)
            :strPtr_(nullptr), size_(0), capacity_(0)
        {
            __init_ptr(static_cast<pointer> (_str.strPtr_), _pos);
        }
        basic_string(const basic_string& _str, size_type _pos, size_type _n)
            :strPtr_(nullptr), size_(0), capacity_(0)
        {
            __init_ptr(static_cast<pointer> (_str.strPtr_), _pos, _n);
        }

        basic_string(const_pointer _strPtr)
            :strPtr_(nullptr), size_(0), capacity_(0)
        {
            __init_ptr(_strPtr);
        }
        basic_string(const_pointer _strPtr, size_type _n)
            :strPtr_(nullptr), size_(0), capacity_(0)
        {
            __init_ptr(_strPtr, 0, _n);
        }

        template <class InputIter, 
                   typename LT::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
        basic_string(InputIter _itBeg, InputIter _itEnd)
        {
            __init_iter(_itBeg, _itEnd);
        }

        basic_string(const basic_string& __rhs)
            :strPtr_(nullptr), size_(0), capacity_(0)
        {
            __init_ptr(__rhs.strPtr_);
        }
        basic_string(basic_string&& __rhs)
            :strPtr_(__rhs.strPtr_), size_(__rhs.size_), capacity_(__rhs.capacity_)
        {
            __rhs.strPtr_ = nullptr;
            __rhs.size_ = 0;
            __rhs.capacity_ = 0;
        }

        basic_string& operator=(const basic_string& __rhs)
        {
            __init_ptr(__rhs.strPtr_);
        }
        basic_string& operator=(basic_string&& __rhs)
        {
            strPtr_ = __rhs.strPtr_;
            size_ = __rhs.size_;
            capacity_ = __rhs.capacity_;
            __rhs.strPtr_ = nullptr;
            __rhs.capacity_ = 0;
            __rhs.size_ = 0;

            return *this;
        }

        basic_string& operator=(const_pointer _strPtr)
        {
            __init_ptr(_strPtr);
        }
        basic_string& operator=(const value_type _ch)
        {
            __init_value(1,_ch);
        }

        ~basic_string()
        { 
            __deallocate_one_str(strPtr_, capacity_);//讲道理这都要析构了，应该是不用调整已有的几个成员变量了，因为马上就回收了
        }

        //-------------------------------------------迭代器相关------------------------------------------------------
        iterator begin() { return strPtr_; }
        const_iterator begin() const { return strPtr_; }
        iterator end() { return strPtr_ + size_; }
        const_iterator end() const { return strPtr_ + size_; }
        reverse_iterator rbegin() { return reverse_iterator(strPtr_ + size_); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(strPtr_ + size_); }
        reverse_iterator rend() { return reverse_iterator(strPtr_); }
        const_reverse_iterator rend() const { return const_reverse_iterator(strPtr_); }
        const_iterator cbegin() const { return strPtr_; }
        const_iterator cend() const { return strPtr_ + size_; }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(strPtr_ + size_); }
        const_reverse_iterator crend() const { return const_reverse_iterator(strPtr_); }

        //容量相关
        size_type size() const { return size_; }
        size_type max_size()const { return static_cast<size_type>(-1); }
        size_type length() const { return size(); }
        size_type capacity() const { return capacity_; }
        void clear() {
            size_ = 0;
            __set_tail_zero(0);
        }
        bool empty() const { return !size_; }
        void resize(size_type _n) { __resize(_n, value_type()); }
        void resize(size_type _n, value_type _value) { __resize(_n, _value); }
        void reserve(size_type _n = 0)
        {
            if(_n > capacity_)
            {
                pointer newPtr = __deallocate_one_str(_n);
                __assign(newPtr, strPtr_, size_);
                LT::swap(newPtr, strPtr_);
                __deallocate_one_str(newPtr);
            }
        }
        void shrink_to_fit() 
        {
            pointer oldStr = nullptr;
            LT::swap(oldStr, strPtr_);
            __init_ptr(oldStr, 0, size_);
            __deallocate_one_str(oldStr, capacity_);
            capacity_ = size_;
        }


        //访问元素相关
        reference operator[] (size_type _pos) { return *(strPtr_ + _pos); }
        const_reference operator[] (size_type _pos) const { return *(strPtr_ + _pos); }
        reference at (size_type _pos) { return *(strPtr_ + _pos); }
        const_reference at (size_type _pos) const { return *(strPtr_ + _pos); }

        reference back() { return *(strPtr_ + size_ - 1); }
        const_reference back() const { return *(strPtr_ + size_ - 1); }
        reference front() { return *(strPtr_); }
        const_reference front() const { return *(strPtr_); }

        void push_back(value_type _value) { __append(1,_value); }
        iterator insert(size_type _pos, const basic_string& _str) { return __insert(_pos, _str.strPtr_, _str.size_); }
        iterator insert(size_type _pos, const basic_string& _str, size_type _subpos, size_type _sublen)
        {
            return __insert(_pos, _str.strPtr_ + _subpos, _sublen);
        }
        iterator insert(size_type _pos, const_pointer _str)
        {
            return __insert(_pos, _str, char_traits::length(_str));
        }
        iterator insert(size_type _pos, const_pointer _ptr, size_type _n)
        {
            return __insert(_pos, _ptr, _n);
        }
        iterator insert(size_type _pos, size_type _n, value_type _value)
        {
            return __insert(_pos, _n, _value);
        }
        iterator insert(iterator _it, size_type _n, value_type _value)
        {
            return __insert(_it, _n, _value);
        }

        iterator insert(iterator _it, value_type _value)
        {
            return __insert(_it, 1, _value);
        }
        template <class InputIter,
                   typename LT::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
            iterator insert(iterator _it, InputIter _itBeg, InputIter _itEnd)
        {
            return __insert(_it, _itBeg, LT::distance(_itBeg, _itEnd));
        }
        basic_string& append(const basic_string& _str)
        {
            __append(_str.strPtr_, _str.strPtr_.size_);
            return *this;
        }
        basic_string& append(const basic_string& _str, size_type _subpos)
        {
            __append(_str.strPtr_ + _subpos, _str.size_);
            return *this;
        }
        basic_string& append(const basic_string& _str, size_type _subpos, size_type _sublen)
        {
            __append(_str.strPtr_ + _subpos, _sublen);
            return *this;
        }
        basic_string& append(const_pointer _ptr)
        {
            __append(_ptr, char_traits::length(_ptr));
            return *this;
        }
        basic_string& append(const_pointer _ptr, size_type _n)
        {
            __append(_ptr, _n);
            return *this;
        }
        basic_string& append(size_type _n, value_type _value)
        {
            __append(_n, _value);
            return *this;
        }
        template <class InputIter,
            typename LT::enable_if<LT::is_input_iterator<InputIter>::value, int>::type = 0>
        basic_string& append(InputIter _itBeg, InputIter _itEnd)
        {
            __append(_itBeg, LT::distance(_itBeg, _itEnd));
            return *this;
        }

        basic_string& operator+= (const basic_string& _str)
        {
            __append(_str.strPtr_, _str.strPtr_.size_);
            return *this;
        }
        basic_string& operator+= (const_pointer _ptr)
        {
            __append(_ptr, char_traits::length(_ptr));
            return *this;
        }
        basic_string& operator+= (value_type _value)
        {
            __append(1, _value);
            return *this;
        }

        void pop_back()
        {
            --size_;
            __set_tail_zero(size_);
        }
        basic_string& erase(size_type _pos, size_type _len)
        {

        }
        iterator erase(iterator _it)
        {
            if (_it != strPtr_ + size_)
            {
                __assign_move(_it, _it + 1, size_ - (_it - strPtr_));
                --size_;
                __set_tail_zero(size_);
            }
            return _it;
        }
        iterator erase(iterator _itBeg, iterator _itEnd)
        {
            
            if (_itBeg != _itEnd) {
                size_type eraseSize = static_cast<size_type>(_itEnd - _itBeg);
                __insert(_itBeg, _itEnd, size_ - (_itEnd - strPtr_));
                size_ -= eraseSize;
                __set_tail_zero(size_);
                
            }
            return _itBeg;
        }
    
        basic_string& replace(size_type _pos, size_type _len, const basic_string& _str)
        {
            return __replace_by_str(strPtr_ + _pos, _len, _str.strPtr_, _str.size_);
        }
        basic_string& replace(iterator _startPos, iterator _endPos, const basic_string& _str)
        {
            return __replace_by_str(_startPos, static_cast<size_type>(_endPos - _startPos), _str.strPtr_, _str.size_);
        }
        basic_string& replace(size_type _pos, size_type _len, const basic_string& _str, size_type _subPos, size_type _sublen = npos)
        {
            return __replace_by_str(_pos, _len, _str.strPtr_ + _subPos, _sublen);
        }
        basic_string& replace(size_type _pos, size_type _len, const_pointer _ptr)
        {
            return __replace_by_str(_pos, _len, _ptr, char_traits::length(_ptr));
        }
        basic_string& replace(iterator _startPos, iterator _endPos, const_pointer _ptr)
        {
            return __replace_by_str(_startPos, static_cast<size_type>(_endPos - _startPos), _ptr, char_traits::length(_ptr));
        }
        basic_string& replace(size_type _pos, size_type _len, const_pointer _ptr, size_type _n)
        {
            return __replace_by_str(strPtr_ + _pos, _len, _ptr, _n);
        }
        basic_string& replace(iterator _startPos, iterator _endPos, const_pointer _ptr, size_type _n)
        {
            return __replace_by_str(strPtr_ + _startPos, static_cast<size_type>(_endPos - _startPos), _ptr, _n);
        }
        basic_string& replace(size_type _pos, size_type _len, size_type _n, value_type _value)
        {
            return __replace_by_n(strPtr_ + _pos, _len, _value, _n);
        }
        basic_string& replace(iterator _startPos, iterator _endPos, size_type _n, value_type _value)
        {
            __replace_by_n(strPtr_ + _startPos, static_cast<size_type>(_endPos - _startPos), _value, _n);
        }
        template <class InputIter>
        basic_string& replace(iterator _startPos, iterator _endPos, InputIter _itBeg, InputIter _itEnd)
        {
            __replace_by_iter(_startPos, static_cast<size_type>(_endPos - _startPos), _itBeg, _itEnd);
        }

        void swap(basic_string& _str) {
            LT::swap(strPtr_, _str.strPtr_);
            LT::swap(size_, _str.size_);
            LT::swap(capacity_, _str.capacity_);
            _str.__set_tail_zero(_str.size_);
            __set_tail_zero(size_);
        }
        
        //count
        size_type count(value_type _ch, size_type _pos = 0)
        {
            size_type ret = 0;
            for (size_type i = _pos; i < size_; ++i)
            {
                if (*(strPtr_ + _pos) == _ch)
                {
                    ++ret;
                }
            }
            return ret;
        }
        size_type find(const basic_string& _str, size_type _pos = 0) const
        {
            const size_type _strLen = _str.size_;
            if (_strLen == 0)
            {
                return _pos;
            }
            if (_pos + _strLen > size_)
            {
                return npos;
            }
            for (size_type i = _pos; i < size_ - _strLen; ++i)
            {
                if (*(i + strPtr_) = *(_str))
                {
                    size_type j = 1;
                    for (; j < _strLen; ++j)
                    {
                        if (*(strPtr_ + i + j) != *(_str + j))
                        {
                            break;
                        }
                    }
                    if (j == _strLen)
                    {
                        return i;
                    }
                }
            }
            return npos;  
        }
        size_type find(const_pointer _str, size_type _pos = 0) const
        {
            const size_type _strLen = char_traits::length(_str);
            if (_strLen == 0)
            {
                return _pos;
            }
            if (_pos + _strLen > size_)
            {
                return npos;
            }
            for (size_type i = _pos; i < size_ - _strLen; ++i)
            {
                if (*(i + strPtr_) = *(_str))
                {
                    size_type j = 1;
                    for (; j < _strLen; ++j)
                    {
                        if (*(strPtr_ + i + j) != *(_str + j))
                        {
                            break;
                        }
                    }
                    if (j == _strLen)
                    {
                        return i;
                    }
                }
            }
            return npos;
        }
        size_type find(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            if (_n == 0)
            {
                return _pos;
            }
            if (_pos + _n > size_)
            {
                return npos;
            }
            for (size_type i = _pos; i < size_ - _n; ++i)
            {
                if (*(i + strPtr_) = *(_ptr))
                {
                    size_type j = 1;
                    for (; j < _n; ++j)
                    {
                        if (*(strPtr_ + i + j) != *(_ptr + j))
                        {
                            break;
                        }
                    }
                    if (j == _n)
                    {
                        return i;
                    }
                }
            }
            return npos;
        }
        size_type find(value_type _value, size_type _pos = 0) const
        {
            for (size_type i = _pos; i < size_; ++i)
            {
                if (*(strPtr_ + i) == _value)
                {
                    return i;
                }
            }
            return npos;
        }

        size_type rfind(const basic_string& _str, size_type _pos = npos) const
        {
            return rfind(_str.strPtr_, npos, _str.size_);
        }
        size_type rfind(const_pointer _ptr, size_type _pos = npos) const
        {
            size_type pos = max(size_ - 1, _pos);
            size_type strSize = char_traits::length(_ptr);
            if(strSize == 0)
            {
                return pos;
            }
            if (pos + 1< strSize)
            {
                return npos;
            }
            for (int i = pos; i >= strSize - 1; --i)
            {
                if (*(_ptr + strSize - 1) == *(strPtr_ + i))
                {
                    size_type j = 1;
                    for (; j < strSize; ++j) {
                        if (*(_ptr + strSize - j - 1) != *(strPtr_ + i - j))
                        {
                            break;
                        }
                    }
                    if (j == strSize) {
                        return i - j + 1;
                    }
                }
            }

            return npos;
        }
        size_type rfind(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            if (_n == 0)
            {
                return _pos;
            }
            if (_pos + _n > size_)
            {
                return npos;
            }
            for (int i = _pos + _n - 1; i >= _n - 1; --i)
            {
                if (*(strPtr_ + i) == *(_ptr + _n - 1))
                {
                    size_type j = 1;
                    for (int j = 1; j < _n; ++j)
                    {
                        if (*(strPtr_ + i - j) != *(_ptr + _n - 1 - j))
                        {
                            break;
                        }
                    }
                    if (j == _n)
                    {
                        return i - j + 1;
                    }
                }
            }

            return npos;
        }
        size_type rfind(value_type _value, size_type _pos = npos) const
        {
            size_type pos = min(npos, size_ - 1);
            for (size_type i = pos; i >= 0; --i)
            {
                if (*(strPtr_ + i) == _value) {
                    return i;
                }
            }
            return npos;
        }

        size_type find_first_of(const basic_string& _str, size_type _pos = 0) const
        {
            return __find_first_of(strPtr_, _str.size_, _pos);
            
        }
        size_type find_first_of(const_pointer _ptr, size_type _pos = 0) const
        {
            return __find_first_of(_ptr, char_traits::length(_ptr), _pos);
        }
        size_type find_first_of(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            return __find_first_of(_ptr, _n, _pos);
        }
        size_type find_first_of(value_type _value, size_type _pos = 0) const
        {
            return find(_value, _pos);
        }
        size_type find_last_of(const basic_string& _str, size_type _pos = npos) const
        {
            return __find_last_of(_str.strPtr_, _str.size_, _pos);
        }
        size_type find_last_of(const_pointer _ptr, size_type _pos = npos) const
        {
            return __find_last_of(_ptr, char_traits::length(_ptr), _pos);
        }
        size_type find_last_of(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            return __find_last_of(_ptr, _n, _pos);
        }
        size_type find_last_of(value_type _value, size_type _pos = npos) const
        {
            for (int i = min(size_ - 1,_pos); i >= 0; --i)
            {
                if (*(strPtr_ + i) == _value) {
                    return i;
                }
            }
            return npos;
        }
        size_type find_first_not_of(const basic_string& _str, size_type _pos = 0) const
        {
            return __find_first_not_of(strPtr_, _str.size_, _pos);

        }
        size_type find_first_not_of(const_pointer _ptr, size_type _pos = 0) const
        {
            return __find_first_not_of(_ptr, char_traits::length(_ptr), _pos);
        }
        size_type find_first_not_of(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            return __find_first_not_of(_ptr, _n, _pos);
        }
        size_type find_first_not_of(value_type _value, size_type _pos = 0) const
        {
            for (; _pos < size_; ++_pos)
            {
                if (*(strPtr_ + _pos) != _value) {
                    return _pos;
                }
            }
            return npos;
        }

        size_type find_last_not_of(const basic_string& _str, size_type _pos = npos) const
        {
            return __find_last_not_of(_str.strPtr_, _str.size_, _pos);
        }
        size_type find_last_not_of(const_pointer _ptr, size_type _pos = npos) const
        {
            return __find_last_not_of(_ptr, char_traits::length(_ptr), _pos);
        }
        size_type find_last_not_of(const_pointer _ptr, size_type _pos, size_type _n) const
        {
            return __find_last_not_of(_ptr, _n, _pos);
        }
        size_type find_last_not_of(value_type _value, size_type _pos = npos) const
        {
            for (int i = min(size_ - 1, _pos); i >= 0; --i)
            {
                if (*(strPtr_ + i) != _value) {
                    return i;
                }
            }
            return npos;
        }

        basic_string substr(size_type _pos = 0, size_type _len = npos) const {
            _pos = min(size_ - 1, _pos);
            _len = min(_len, size_ - _pos + 1);
            return basic_string(begin() + _pos, begin() + _pos + _len);
        }

        int compare(const basic_string& _str) const
        {
            return __compare_pointer(strPtr_, size_, _str.strPtr_, _str.size_);
        }
        int compare(size_type _pos, size_type _len, const basic_string& _str) const
        {
            return __compare_pointer(strPtr_, LT::min(size_ - _pos, _len), _str.strPtr_, _str.size_);
        }
        int compare(size_type _pos, size_type _len, const basic_string& _str,
            size_type _subpos, size_type _sublen = npos) const
        {
            return __compare_pointer(strPtr_, LT::min(size_ - _pos, _len),
                   _str.strPtr_ + _subpos, LT::min(_str.size_ - _subpos, _sublen));
        }
        int compare(const_pointer _ptr) const
        {
            return __compare_pointer(strPtr_, size_, _ptr, char_traits::length(_ptr));
        }
        int compare(size_type _pos, size_type _len, const_pointer _ptr) const
        {
            return __compare_pointer(strPtr_, LT::min(size_ - _pos, _len), _ptr, char_traits::length(_ptr));
        }
        int compare(size_type _pos, size_type _len, const_pointer _ptr, size_type _n) const
        {
            return __compare_pointer(static_cast<const_pointer>(strPtr_), LT::min(size_ - _pos, _len), _ptr, min(char_traits::length(_ptr), _n));
        }
        
        //反转

        void reverse()
        {
            if (size_ < 2) { return; }
            pointer left = strPtr_;
            pointer right = strPtr_ + size_ - 1;
            while (left < right)
            {
                iter_swap(left, right);
                ++left;
                --right;
            }
        }
        //与c字符串兼容
        const_pointer c_str()const { return strPtr_; }
        const_pointer data()const { return strPtr_; }
        size_type copy(pointer _dst, size_type _len, size_type _pos = 0)
        {
            assert(_len + _pos < size_);
            size_type copySize = LT::min(size_ - _pos, _len);
            pointer ptr = __assign_move(_dst, strPtr_ + _pos, copySize);
            return copySize;
        }
        
        //操作符重载
        // 重载 operator+= 
        basic_string& operator+=(const basic_string& _rhs)
        {
            return append(_rhs);
        }
        basic_string& operator+=(value_type _ch)
        {
            return append(1, _ch);
        }
        basic_string& operator+=(const_pointer _ptr)
        {
            return append(_ptr, _ptr + char_traits::length(_ptr));
        }

        // 重载 operator >> / operatror <<
        friend std::istream& operator>> (std::istream& _inputStream, basic_string& _str)
        {
            value_type* buffer = new value_type[4096];
            _inputStream >> buffer;
            basic_string tmp(buffer);
            _str = std::move(tmp);
            delete[]buffer;
            return _inputStream;
        }

        friend std::ostream& operator << (std::ostream& _outputStream, const basic_string& _str)
        {
            for (size_type i = 0; i < _str.size(); ++i)
            {
                _outputStream << *(_str.strPtr_ + i);
            }
            return _outputStream;
        }
        /////********************************************************************************************************
        //**************************************************内部实现**************************************************
        ////**********************************************************************************************************
        private:
        //------------------------------------------------内存相关.--------------------------------------------
        //因为char，wchar_t,char16_t,char32_t 都是基本数据类型，所以无需构造，只需要获取一块内存空间即可
        typedef  Alloc  data_allocator;
        #define INIT_SIZE 15
        #define EXPAN_RATIO  1.5
        data_allocator get_allocator()const
        {
            return data_allocator();
        }
        //申请一块内存，大小为n个value_type
        //内存不足则可能返回空指针，但是其实应该不会，因为正常情况下内存不足内存分配器会抛出bad_allco类型异常，
        //用set_new_handler()指定一个自定义函数去处理内存不足问题，之后，分配内存不足时，不会默认抛出bad_allco()异常
        //之所以这么设置是为了保证在某些内存策略中，可能存在强制不抛出异常，内存不足返回空指针的情况
        inline pointer __allocate_one_str_maybe_nullptr(size_type _n)
        {
            //自己重新包装分配和回收函数的好处就在于此，如果需要改动对于string的分配策略，可以仅仅改变接口
            pointer mem =  data_allocator::allocate(_n + 1);//多加的一位是为了保证与c兼容
            if (mem) {
                __set_tail_zero(mem, _n);
            }
            return mem;
        }
        //该函数负责保证不返回空指针，一定分配成功内存，否则就中断程序，即使传入0，那么也至少分配一个一个value_type的内存空间
       inline pointer __allocate_one_str(size_type _n) 
       {
          pointer memPtr = __allocate_one_str_maybe_nullptr(_n);
          assert(memPtr, "failded to allocate new memory!");
          return memPtr;
       }
          
       //回收一块大小为_n字节的内存，仅仅在内存分配器需要获知该内存大小的时候有用，因为默认的内存回收free只需要传入指针即可
       inline void __deallocate_one_str(pointer _ptr, size_type _n)
       {
           data_allocator::deallocate(_ptr, (_n + 1) * sizeof(value_type));
       }

       //为一块内存赋值
       //该函数只负责赋值，为了性能，是否溢出留给上一层函数判断
       inline void __assign(pointer _writePtr, size_type _n, value_type _value) {
           char_traits::fill(_writePtr, _value, _n);
       }

       //调用该函数允许发生重叠
       template<class InputIter>
       inline void __assign_move(pointer _writePtr, InputIter _itBeg, size_type _n) {
           for (; _n; ++_writePtr, ++_itBeg) {
               *_writePtr = *_itBeg;
           }
       }
       template<>//特化
       inline void __assign_move<const_pointer>(pointer _writePtr, const_pointer _itBeg, size_type _n) {
           char_traits::move(_writePtr, _itBeg, _n);
       }

       //调用该函数要保证不发生内存区域重叠
       template<class InputIter>
       inline void __assign_copy(pointer _writePtr, InputIter _itBeg, size_type _n) {
           //可能指向是同一个内存
           auto addr = LT::address_of(*_itBeg);
           if (_writePtr < addr) {
               for (; _n; ++_writePtr, ++_itBeg) {
                   *_writePtr = *_itBeg;
               }
           }
           else if (_writePtr > addr) {
               for (_itBeg += _n, _writePtr += _n; _n; --_n, --_itBeg, --_writePtr) {
                   *_writePtr = *_itBeg;
               }
           }
           
       }

       template<>//特化
       inline void __assign_copy<const_pointer>(pointer _writePtr, const_pointer _itBeg, size_type _n) {
           char_traits::copy(_writePtr, _itBeg, _n);
       }

       //扩展内存，该函数会自己判断是否需要扩容，如果需要扩容，那么会自动扩容到需要的大小,并拷贝原有区域
       void __expand_capacity(size_type _newSize)
       {
           //注意这里是有整数溢出风险的，但是，几乎不可能出现，所以为了性能，不添加是否溢出的检查
           size_type needSize = _newSize;
           size_type newCap = capacity_;
           while (newCap < needSize) {
               newCap *= EXPAN_RATIO;//这里面1.5是扩容系数，如果需要人工改变的话，需要提供接口
           }
           if (newCap > capacity_)//用大于而不是等于进行判断一定程度可以防止整型溢出，但是很有限
           {
               pointer newPtr = __allocate_one_str(newCap);//由分配函数保证不为空指针
               if (size_)
               {
                   __assign_copy(newPtr, strPtr_, size_);//如果原来不是空string
               }
               LT::swap(newPtr, strPtr_);
               LT::swap(capacity_, newCap);
               __deallocate_one_str(newPtr,newCap);
           }  
       }

       //调用此函数的前提是，原有size >= _preSize
       //该函数会自动进行扩容，并且无论是否扩容，都会在第_preSize个位置(0开始计数)开始，留出_midn个空白位置。
       void __expand_capacity_midn(size_type _newSize, size_type _preSize, size_type _subSize, size_type _midn)
       {
           assert(_preSize + _subSize <= size_);
           //注意这里是有整数溢出风险的，但是，几乎不可能出现，所以为了性能，不添加是否溢出的检查
           size_type needSize = _newSize;
           size_type newCap = capacity_;
           while (newCap < needSize) {
               newCap *= EXPAN_RATIO;//这里面1.5是扩容系数，如果需要人工改变的话，需要提供接口
           }
           if (newCap > capacity_)//用大于而不是等于进行判断一定程度可以防止整型溢出，但是很有限
           {
               pointer newPtr = __allocate_one_str(newCap);//由分配函数保证不为空指针
               if (_preSize)
               {
                   __assign_copy(newPtr, strPtr_, _preSize);
                   __assign_copy(newPtr + _preSize + _midn, strPtr_ + _preSize + _subSize, size_ - _preSize - _subSize);
               }
               LT::swap(newPtr, strPtr_);
               LT::swap(capacity_, newCap);
               __deallocate_one_str(newPtr, newCap);
              
           }
           else {
               //无需扩容
               if (_preSize < size_)
               {
                   __assign_move(strPtr_ + _preSize + _midn, strPtr_ + _preSize + _subSize, size_ - _preSize - _subSize);
               }
              
           }
           size_ += _midn;
           __set_tail_zero(size_);
       }
       //实现与c兼容：
       inline void __set_tail_zero(size_type _size)
       {
           char_traits::fill(strPtr_ + _size, 0, 1);
       }

       inline void __set_tail_zero(pointer _ptr, size_type _size)
       {
           char_traits::fill(_ptr + _size, 0, 1);
       }
       //初始化- 
       inline void __init()
       {
           capacity_ = INIT_SIZE;//默认留出15个字节空间
           __allocate_one_str_maybe_nullptr(capacity_);
           if (strPtr_ == nullptr) {
               capacity_ = 0;
           }
       }

       //产生n个_value字符
       inline void __init_value(size_type _n, value_type _value) {
           try {
               __expand_capacity(_n);
               __assign(strPtr_, _n, _value);
               size_ = _n;
               __set_tail_zero(size_);
           }
           catch (...) {
               strPtr_ = nullptr;
               size_ = 0;
               capacity_ = 0;
           }
          
       }

       //用传入的指针进行初始化
       void __init_ptr(const_pointer _ptr, size_type _pos) {
           try {
               size_type newSize = static_cast<size_type>(char_traits::length(_ptr + _pos));
               __expand_capacity(newSize);
               __assign_cpy(strPtr_, _ptr + _pos, newSize);
               size_ = newSize;
               __set_tail_zero(newSize);
           }
           catch(...){
               size_ = 0;
               strPtr_ = nullptr;
               capacity_  = 0;
           }   
       }

       void __init_ptr(const_pointer _ptr, size_type _pos, size_type _n) {
           try {
               size_type len = static_cast<size_type>(char_traits::length(_ptr + _pos));
               size_type newSize = min(len, _n);
               __expand_capacity(newSize);
               __assign_cpy(strPtr_, _ptr + _pos, newSize);
               size_ = newSize;
               __set_tail_zero(newSize);
           }
           catch(...){
               size_ = 0;
               strPtr_ = nullptr;
               capacity_  = 0;
           }
       }

       template<class InputIter>
       void __init_iter(InputIter _itBeg, InputIter _itEnd) {
           try {
               size_type newSize = static_cast<size_type>(LT::distance(_itBeg, _itEnd));
               __expand_capacity(newSize);
               __assign_cpy(strPtr_, _itBeg, newSize);
               size_ = newSize;
               __set_tail_zero(size_);
           }
           catch (...) {
               size_ = 0;
               strPtr_ = nullptr;
               capacity_ = 0;
           }
       }

       //--------------------------------接口实现-------------------------------------------------------

       //resize
        void __resize(size_type _n, value_type _value)
       {
            if (_n < size_) {
                size_ = _n;
                __set_tail_zero(size_);
            }
            else {
                append(_n - size_, _value);
            }
       }

        //insert:在指定位置插入n个元素
        iterator __insert(const_iterator _pos, size_type _n, value_type _value)
        {
            iterator end = end();
            iterator pos = static_cast<iterator>(_pos);

            if (end == pos) {
                iterator ret = strPtr_ + size_;
                __append(_n, _value);
                return ret;
            }
            else {
                //可以直接减的，因为确定是连续内存
                size_type preSize = static_cast<size_type>(_pos - strPtr_);
                __expand_capacity_midn(_n + size_,preSize, _n);
                __assign(strPtr_ + preSize, _n, _value);
                return strPtr_ + preSize;
            }
        }

        iterator __insert(const_iterator _pos, const_iterator _readPtr, size_type _n)
        {
            iterator end = end();
            iterator pos = static_cast<iterator> (_pos);

            if (end == pos) {
                iterator ret = strPtr_ + size_;
                __append(_readPtr, _n);
                return ret;
            }
            else {
                //可以直接减的，因为确定是连续内存
                size_type preSize = static_cast<size_type>(_pos - strPtr_);
                __expand_capacity_midn(_n + size_, preSize, _n);
                __assign_cpy(strPtr_ + preSize, _readPtr, _n);
                iterator ret = strPtr_ + preSize;
                return ret;
            }
        }

        //append:在末尾追加n个元素
        void __append(size_type _n, value_type _value)
        {
            __expand_capacity(_n + size_);
            __assign(end(), _n, _value);
            iterator ret = strPtr_ + size_;
            size_ += _n;
            __set_tail_zero(size_);
            return ret;
        }

        //append:在末尾追加一个长度为n的字符串
        void __append(const_iterator _writePtr, size_type _n)
        {
            __expand_capacity(_n + size_);
            __assign(strPtr_ + size_, _writePtr, _n);
            iterator ret = strPtr_ + size_;
            size_ += _n;
            __set_tail_zero(size_);
            return ret;
        }

        //replace: 用新的字符替代原有字符
        inline basic_string& __replace_by_n(const_iterator _posIt, size_type _len, value_type _value, size_type _n)
        {
            //该函数把从posIt位置开始的_len个字符替换成n个value字符
            assert(_posIt <= cend() && _posIt >= cbegin());
            size_type preSize = static_cast<size_type>(const_cast<iterator>(_posIt) - strPtr_);
            size_type subSize = min(static_cast<size_type>(_posIt - strPtr_), _len);
            size_type needCap = size_ - subSize + _n;
            
            __expand_capacity_midn(needCap, preSize, subSize, _n);
            __assign(strPtr_ + preSize, _n, _value);
            return *this;
        }

        inline basic_string& __replace_by_str(const_iterator _posIt, size_type _len, const_pointer _str, size_type _n)
        {
            //该函数把从posIt位置开始的_len个字符替换成str字符串的n个字符
            assert(_posIt <= cend() && _posIt >= cbegin());
            size_type addSize = min(static_cast<size_type>(char_traits::length(_str)), _n);
            size_type subSize = min(static_cast<size_type>(strPtr_ - const_cast<iterator>(_posIt)), _len);
            __expand_capacity_midn(size_ + addSize - subSize, static_cast<size_type>(_posIt - strPtr_), subSize, _n);
            __assign_copy(strPtr_, _str, _n);
            return *this;
        }   

        template<class InputIter>
        inline basic_string& __replace_by_iter(const_iterator _posIt, size_type _len, InputIter _itBeg, InputIter _itEnd)
        {
            //该函数把从posIt位置开始的_len个字符替换成str字符串的n个字符
            assert(_posIt <= cend() && _posIt >= cbegin());
            size_type addSize = LT::distance(_itBeg, _itEnd);
            size_type subSize = min(static_cast<size_type>(strPtr_ - const_cast<iterator>(_posIt)), _len);
            __expand_capacity_midn(size_ + addSize - subSize, static_cast<size_type>(_posIt - strPtr_), subSize, addSize);
            __assign_copy(const_cast<iterator>(_posIt), _itBeg, addSize);
            return *this;
        }

        //find_first_of
        inline size_type __find_first_of(const_pointer _str, size_type _n, size_type _pos) const
        {
            for (int i = _pos; i < size_; ++i)
            {
                value_type ch = *(_pos + strPtr_);
                for (size_type j = 0; j < _n; ++j)
                {
                    if (*(_str + j) == ch) {
                        return i;
                    }
                }
            }
            return npos;
        }

        //find_last_of
        inline size_type __find_last_of(const_pointer _str, size_type _n, size_type _pos) const
        {
            for (int i = _pos; i > size_; --i)
            {
                value_type ch = *(_pos + strPtr_);
                for (size_type j = 0; j < _n; ++j)
                {
                    if (*(_str + j) == ch) {
                        return i;
                    }
                }
            }
            return npos;
        }

        //find_first_not_of
        inline size_type __find_first_not_of(const_pointer _str, size_type _n, size_type _pos) const
        {
            for (int i = _pos; i < size_; ++i)
            {
                value_type ch = *(_pos + strPtr_);
                for (size_type j = 0; j < _n; ++j)
                {
                    if (*(_str + j) == ch) {
                        break;
                    }
                }
                return i;
            }
            return npos;
        }

        //find_first_not_of
        inline size_type __find_last_not_of(const_pointer _str, size_type _n, size_type _pos)const
        {
            for (int i = _pos; i > size_; --i)
            {
                value_type ch = *(_pos + strPtr_);
                for (size_type j = 0; j < _n; ++j)
                {
                    if (*(_str + j) == ch) {
                        break;
                    }
                }
                return i;
            }
            return npos;
        }

        //compare的实现:如果相等则输出为0,小于返回-1，大于返回1。
        inline int __compare_pointer(const_pointer _str1, size_type _n1,
            const_pointer _str2, size_type _n2) const
        {
            size_type len = min(_n1, _n2);
            int ret = char_traits::compare(_str1, _str2, len);
            if (ret != 0) { return ret; }
            if (_n1 < _n2) { return -1; }
            if (_n1 > _n2) { return 1; }
            return 0;
        }

        inline int __compare_pointer(pointer _str1, size_type _n1,
            const_pointer _str2, size_type _n2) const
        {
            size_type len = min(_n1, _n2);
            int ret = char_traits::compare(_str1, _str2, len);
            if (ret != 0) { return ret; }
            if (_n1 < _n2) { return -1; }
            if (_n1 > _n2) { return 1; }
            return 0;
        }
	};


    //-------------------------------------------外部重载-----------------------------------------------------
    // 重载 operator+
    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const basic_string<CharType, CharTraits, Alloc>& _lhs,
            const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(_lhs);
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const CharType* _lhs, const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(_lhs);
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(CharType _ch, const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(1, _ch);
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const basic_string<CharType, CharTraits, Alloc>& _lhs, const CharType* _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(_lhs);
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const basic_string<CharType, CharTraits,Alloc>& _lhs, CharType _ch)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(_lhs);
        tmp.append(1, _ch);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(basic_string<CharType, CharTraits, Alloc>&& _lhs,
            const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_lhs));
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const basic_string<CharType, CharTraits, Alloc>& _lhs,
            basic_string<CharType, CharTraits, Alloc>&& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_rhs));
        tmp.insert(tmp.begin(), _lhs.begin(), _lhs.end());
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(basic_string<CharType, CharTraits, Alloc>&& _lhs,
            basic_string<CharType, CharTraits, Alloc>&& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_lhs));
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(const CharType* _lhs, basic_string<CharType, CharTraits, Alloc>&& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_rhs));
        tmp.insert(tmp.begin(), _lhs, _lhs + char_traits<CharType>::length(_lhs));
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(CharType _ch, basic_string<CharType, CharTraits, Alloc>&& _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_rhs));
        tmp.insert(tmp.begin(), _ch);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(basic_string<CharType, CharTraits, Alloc>&& _lhs, const CharType* _rhs)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_lhs));
        tmp.append(_rhs);
        return tmp;
    }

    template <class CharType, class CharTraits, class Alloc>
    basic_string<CharType, CharTraits, Alloc>
        operator+(basic_string<CharType, CharTraits, Alloc>&& _lhs, CharType _ch)
    {
        basic_string<CharType, CharTraits, Alloc> tmp(LT::move(_lhs));
        tmp.append(1, _ch);
        return tmp;
    }

    // 重载比较操作符
    template <class CharType, class CharTraits, class Alloc>
    bool operator==(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.size() == _rhs.size() && _lhs.compare(_rhs) == 0;
    }

    template <class CharType, class CharTraits, class Alloc>
    bool operator!=(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.size() != _rhs.size() || _lhs.compare(_rhs) != 0;
    }

    template <class CharType, class CharTraits, class Alloc>
    bool operator<(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.compare(_rhs) < 0;
    }

    template <class CharType, class CharTraits, class Alloc>
    bool operator<=(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.compare(_rhs) <= 0;
    }

    template <class CharType, class CharTraits, class Alloc>
    bool operator>(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.compare(_rhs) > 0;
    }

    template <class CharType, class CharTraits, class Alloc>
    bool operator>=(const basic_string<CharType, CharTraits, Alloc>& _lhs,
        const basic_string<CharType, CharTraits, Alloc>& _rhs)
    {
        return _lhs.compare(_rhs) >= 0;
    }

    // 重载 LT 的 swap
    template <class CharType, class CharTraits, class Alloc>
    void swap(basic_string<CharType, CharTraits, Alloc>& _lhs,
        basic_string<CharType, CharTraits, Alloc>& _rhs) 
    {
        _lhs.swap(_rhs);
    }

    // string必须自己重写hash函数，因为这是自定义的数据类型
    template <class CharType, class CharTraits, class Alloc>
    struct hash<basic_string<CharType, CharTraits, Alloc>>
    {
        size_t operator()(const basic_string<CharType, CharTraits, Alloc>& _str)
        {
            return bitwise_hash((const unsigned char*)_str.c_str(),
                _str.size() * sizeof(CharType));
        }
    };
}