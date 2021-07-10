//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once

//д��û�뵽��ԭ��string�Ĵ�������ô��

//��ͷ�ļ��ṩSTLʵ�ֵ��ַ�����Ļ�������,���ݲ�����MSVC�ṩ�Ĳ�����ͬ
//�������һ�¸�string�����ݲ���:��ʼ������15(��ʵռ�ݵ��ڴ���16�ֽڣ����һ���ֽ����ڱ���'\0')��Ȼ��ÿ����������ʱ������1.5������
//�����һ���Ը�ֵ����ô����ȡһ���Ը�ֵ�Ĵ�С�����Ǹô�С���ٴ���15
//��ǰ����Ϊm��size Ϊs�����һ���� += һ������Ϊn���ַ���������m < s + n����ô��ʱ������һֱ��1.5����չ��ֱ��capacity() >= s + n;

//�Լ���˼��һ�����ݲ��ԣ������Ӵ�СΪ��С���ַ���ʱ��1.5�����ݣ����������ݵ�s + n + k��kΪ�����
//�����ǽ��java�����ݳ���2��
//��ͬ�����ݲ���ʵ�����������ף�ֻ��Ҫ��д/��д����__expand_capacity()���ɣ����е����ݾ�������һ������//����Ϊ�����������úܿ�ѧ����
//����������ʱû��ȷ���Լ��Ĳ��Ը��ã��Ͱ���׼��ʵ����

//�����ڴ�ʱ�õ��ļ����Ƕ�����һ���ֽڵ�������ÿ�θı�sizeʱ����size��һλ���ó�'\0'
//ԭ��:�ı�size_�ĺ�������set_tail_zero, �ı��ַ������һ���ַ�����λ�õĺ�������ı�size_
//�����Լ�д��ʱ���֣� �Һ�ϲ����д���룬���ڴ��뱸ע�ĵط�д�Լ���ʱ���뷨
//������
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

    //char 8bit�� wchar_t 16bit����32bit; char16_t 16bit, char32_t 32bit;
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
           assert(_src + _n <= _dst || _dst + _n <= _src);//��������Ƿ��غ�
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
		
		//��������һЩ���Ͷ���
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

		//����string�ڲ���������Ҫ��ָ��
	private:
		iterator  strPtr_;
        size_type size_;//Ϊ�˱��ֺ�c���ݣ�ÿ�θı�size_.��Ҫִ��__set_tail_zero(size_);
        size_type capacity_;
    public:
        static constexpr size_type npos = static_cast<size_type>(-1);
	/////********************************************************************************************************
	//**************************************************����ӿ�**************************************************
	////**********************************************************************************************************
    public:
		//-------------------------------------------�������-------------------------------------------------------
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
            __deallocate_one_str(strPtr_, capacity_);//�������ⶼҪ�����ˣ�Ӧ���ǲ��õ������еļ�����Ա�����ˣ���Ϊ���Ͼͻ�����
        }

        //-------------------------------------------���������------------------------------------------------------
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

        //�������
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


        //����Ԫ�����
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
        
        //��ת

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
        //��c�ַ�������
        const_pointer c_str()const { return strPtr_; }
        const_pointer data()const { return strPtr_; }
        size_type copy(pointer _dst, size_type _len, size_type _pos = 0)
        {
            assert(_len + _pos < size_);
            size_type copySize = LT::min(size_ - _pos, _len);
            pointer ptr = __assign_move(_dst, strPtr_ + _pos, copySize);
            return copySize;
        }
        
        //����������
        // ���� operator+= 
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

        // ���� operator >> / operatror <<
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
        //**************************************************�ڲ�ʵ��**************************************************
        ////**********************************************************************************************************
        private:
        //------------------------------------------------�ڴ����.--------------------------------------------
        //��Ϊchar��wchar_t,char16_t,char32_t ���ǻ����������ͣ��������蹹�죬ֻ��Ҫ��ȡһ���ڴ�ռ伴��
        typedef  Alloc  data_allocator;
        #define INIT_SIZE 15
        #define EXPAN_RATIO  1.5
        data_allocator get_allocator()const
        {
            return data_allocator();
        }
        //����һ���ڴ棬��СΪn��value_type
        //�ڴ治������ܷ��ؿ�ָ�룬������ʵӦ�ò��ᣬ��Ϊ����������ڴ治���ڴ���������׳�bad_allco�����쳣��
        //��set_new_handler()ָ��һ���Զ��庯��ȥ�����ڴ治�����⣬֮�󣬷����ڴ治��ʱ������Ĭ���׳�bad_allco()�쳣
        //֮������ô������Ϊ�˱�֤��ĳЩ�ڴ�����У����ܴ���ǿ�Ʋ��׳��쳣���ڴ治�㷵�ؿ�ָ������
        inline pointer __allocate_one_str_maybe_nullptr(size_type _n)
        {
            //�Լ����°�װ����ͻ��պ����ĺô������ڴˣ������Ҫ�Ķ�����string�ķ�����ԣ����Խ����ı�ӿ�
            pointer mem =  data_allocator::allocate(_n + 1);//��ӵ�һλ��Ϊ�˱�֤��c����
            if (mem) {
                __set_tail_zero(mem, _n);
            }
            return mem;
        }
        //�ú�������֤�����ؿ�ָ�룬һ������ɹ��ڴ棬������жϳ��򣬼�ʹ����0����ôҲ���ٷ���һ��һ��value_type���ڴ�ռ�
       inline pointer __allocate_one_str(size_type _n) 
       {
          pointer memPtr = __allocate_one_str_maybe_nullptr(_n);
          assert(memPtr, "failded to allocate new memory!");
          return memPtr;
       }
          
       //����һ���СΪ_n�ֽڵ��ڴ棬�������ڴ��������Ҫ��֪���ڴ��С��ʱ�����ã���ΪĬ�ϵ��ڴ����freeֻ��Ҫ����ָ�뼴��
       inline void __deallocate_one_str(pointer _ptr, size_type _n)
       {
           data_allocator::deallocate(_ptr, (_n + 1) * sizeof(value_type));
       }

       //Ϊһ���ڴ渳ֵ
       //�ú���ֻ����ֵ��Ϊ�����ܣ��Ƿ����������һ�㺯���ж�
       inline void __assign(pointer _writePtr, size_type _n, value_type _value) {
           char_traits::fill(_writePtr, _value, _n);
       }

       //���øú����������ص�
       template<class InputIter>
       inline void __assign_move(pointer _writePtr, InputIter _itBeg, size_type _n) {
           for (; _n; ++_writePtr, ++_itBeg) {
               *_writePtr = *_itBeg;
           }
       }
       template<>//�ػ�
       inline void __assign_move<const_pointer>(pointer _writePtr, const_pointer _itBeg, size_type _n) {
           char_traits::move(_writePtr, _itBeg, _n);
       }

       //���øú���Ҫ��֤�������ڴ������ص�
       template<class InputIter>
       inline void __assign_copy(pointer _writePtr, InputIter _itBeg, size_type _n) {
           //����ָ����ͬһ���ڴ�
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

       template<>//�ػ�
       inline void __assign_copy<const_pointer>(pointer _writePtr, const_pointer _itBeg, size_type _n) {
           char_traits::copy(_writePtr, _itBeg, _n);
       }

       //��չ�ڴ棬�ú������Լ��ж��Ƿ���Ҫ���ݣ������Ҫ���ݣ���ô���Զ����ݵ���Ҫ�Ĵ�С,������ԭ������
       void __expand_capacity(size_type _newSize)
       {
           //ע��������������������յģ����ǣ����������ܳ��֣�����Ϊ�����ܣ�������Ƿ�����ļ��
           size_type needSize = _newSize;
           size_type newCap = capacity_;
           while (newCap < needSize) {
               newCap *= EXPAN_RATIO;//������1.5������ϵ���������Ҫ�˹��ı�Ļ�����Ҫ�ṩ�ӿ�
           }
           if (newCap > capacity_)//�ô��ڶ����ǵ��ڽ����ж�һ���̶ȿ��Է�ֹ������������Ǻ�����
           {
               pointer newPtr = __allocate_one_str(newCap);//�ɷ��亯����֤��Ϊ��ָ��
               if (size_)
               {
                   __assign_copy(newPtr, strPtr_, size_);//���ԭ�����ǿ�string
               }
               LT::swap(newPtr, strPtr_);
               LT::swap(capacity_, newCap);
               __deallocate_one_str(newPtr,newCap);
           }  
       }

       //���ô˺�����ǰ���ǣ�ԭ��size >= _preSize
       //�ú������Զ��������ݣ����������Ƿ����ݣ������ڵ�_preSize��λ��(0��ʼ����)��ʼ������_midn���հ�λ�á�
       void __expand_capacity_midn(size_type _newSize, size_type _preSize, size_type _subSize, size_type _midn)
       {
           assert(_preSize + _subSize <= size_);
           //ע��������������������յģ����ǣ����������ܳ��֣�����Ϊ�����ܣ�������Ƿ�����ļ��
           size_type needSize = _newSize;
           size_type newCap = capacity_;
           while (newCap < needSize) {
               newCap *= EXPAN_RATIO;//������1.5������ϵ���������Ҫ�˹��ı�Ļ�����Ҫ�ṩ�ӿ�
           }
           if (newCap > capacity_)//�ô��ڶ����ǵ��ڽ����ж�һ���̶ȿ��Է�ֹ������������Ǻ�����
           {
               pointer newPtr = __allocate_one_str(newCap);//�ɷ��亯����֤��Ϊ��ָ��
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
               //��������
               if (_preSize < size_)
               {
                   __assign_move(strPtr_ + _preSize + _midn, strPtr_ + _preSize + _subSize, size_ - _preSize - _subSize);
               }
              
           }
           size_ += _midn;
           __set_tail_zero(size_);
       }
       //ʵ����c���ݣ�
       inline void __set_tail_zero(size_type _size)
       {
           char_traits::fill(strPtr_ + _size, 0, 1);
       }

       inline void __set_tail_zero(pointer _ptr, size_type _size)
       {
           char_traits::fill(_ptr + _size, 0, 1);
       }
       //��ʼ��- 
       inline void __init()
       {
           capacity_ = INIT_SIZE;//Ĭ������15���ֽڿռ�
           __allocate_one_str_maybe_nullptr(capacity_);
           if (strPtr_ == nullptr) {
               capacity_ = 0;
           }
       }

       //����n��_value�ַ�
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

       //�ô����ָ����г�ʼ��
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

       //--------------------------------�ӿ�ʵ��-------------------------------------------------------

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

        //insert:��ָ��λ�ò���n��Ԫ��
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
                //����ֱ�Ӽ��ģ���Ϊȷ���������ڴ�
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
                //����ֱ�Ӽ��ģ���Ϊȷ���������ڴ�
                size_type preSize = static_cast<size_type>(_pos - strPtr_);
                __expand_capacity_midn(_n + size_, preSize, _n);
                __assign_cpy(strPtr_ + preSize, _readPtr, _n);
                iterator ret = strPtr_ + preSize;
                return ret;
            }
        }

        //append:��ĩβ׷��n��Ԫ��
        void __append(size_type _n, value_type _value)
        {
            __expand_capacity(_n + size_);
            __assign(end(), _n, _value);
            iterator ret = strPtr_ + size_;
            size_ += _n;
            __set_tail_zero(size_);
            return ret;
        }

        //append:��ĩβ׷��һ������Ϊn���ַ���
        void __append(const_iterator _writePtr, size_type _n)
        {
            __expand_capacity(_n + size_);
            __assign(strPtr_ + size_, _writePtr, _n);
            iterator ret = strPtr_ + size_;
            size_ += _n;
            __set_tail_zero(size_);
            return ret;
        }

        //replace: ���µ��ַ����ԭ���ַ�
        inline basic_string& __replace_by_n(const_iterator _posIt, size_type _len, value_type _value, size_type _n)
        {
            //�ú����Ѵ�posItλ�ÿ�ʼ��_len���ַ��滻��n��value�ַ�
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
            //�ú����Ѵ�posItλ�ÿ�ʼ��_len���ַ��滻��str�ַ�����n���ַ�
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
            //�ú����Ѵ�posItλ�ÿ�ʼ��_len���ַ��滻��str�ַ�����n���ַ�
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

        //compare��ʵ��:�����������Ϊ0,С�ڷ���-1�����ڷ���1��
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


    //-------------------------------------------�ⲿ����-----------------------------------------------------
    // ���� operator+
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

    // ���رȽϲ�����
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

    // ���� LT �� swap
    template <class CharType, class CharTraits, class Alloc>
    void swap(basic_string<CharType, CharTraits, Alloc>& _lhs,
        basic_string<CharType, CharTraits, Alloc>& _rhs) 
    {
        _lhs.swap(_rhs);
    }

    // string�����Լ���дhash��������Ϊ�����Զ������������
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