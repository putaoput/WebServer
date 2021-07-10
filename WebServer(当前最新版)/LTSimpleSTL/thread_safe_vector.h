//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

#include <assert.h>
#include "memory.h"
#include "type_traits.h"
#include "allocator.h"
#include "uninitialized.h"
#include "algobase.h"
#include <atomic>

//该文件利用atomic实现无锁定长循环数组
//对以下接口进行线程安全保证：
//预初始化，支持右值引用和移动以提高效率。
//使用了消费者和生产者模型，通过返回值来提示应该调用消费者还是生产者
//push_back()
//pop_front()
//另外以上函数都有阻塞线程的版本, 确保一定成功

//优化性能，如果对性能有执念，要扣哪怕那么一点点的性能，那么请把LEGNTH参数设置成2的幂，这样可以用移位操作代替取余。

namespace LT {
    typedef  char vector_node_type;
    static constexpr vector_node_type forbbiden_vector_node = 'F';
    static constexpr vector_node_type writable_vector_node = 'W';
    static constexpr vector_node_type readable_vector_node = 'R';

    //static constexpr vector_node_type temp
  
        template<class T>
        struct __thread_safe_vector_node
        {
            vector_node_type nodeType_;
            T data_;
            __thread_safe_vector_node(vector_node_type _nodeType = writable_vector_node, T _data = T())
                :nodeType_(_nodeType),data_(_data) {}
        };
        

    template<typename T, size_t LENGTH, class Alloc = allocator<std::atomic<__thread_safe_vector_node<T>>>>
    class thread_safe_vector {
        //先定义一些类型
    public:
        typedef T											value_type;
        typedef __thread_safe_vector_node<T>                node_type;
        typedef std::atomic<__thread_safe_vector_node<T>>   atomic_node_type;
        typedef node_type*                                  node_pointer;
        typedef atomic_node_type*                           atomic_node_pointer;
        typedef value_type*									pointer;
        typedef const T*                                    const_pointer;
        typedef T&                                          reference;
        typedef const T&                                    const_reference;
        typedef atomic_node_type&                           atomic_node_reference;
        typedef const atomic_node_type&                     const_atomic_node_reference;
        typedef size_t										size_type;
        typedef ptrdiff_t									difference_type;

        //定义vector内部的三个重要的指针
    private:
        atomic_node_pointer start_;
        std::atomic<size_type> front_; //用来标识循环数组的头
        std::atomic<size_type> back_;  //用来标识循环数组的尾
        std::atomic<int> writableCount_; //用来确定任务队列是否为空，或者为满
        std::atomic<int> readableCount_;


/////********************************************************************************************************
//**************************************************对外接口**************************************************
////**********************************************************************************************************
    public:
        //写一些构造函数
         thread_safe_vector()
             :start_(0)
        {
            //对每个元素进行预初始化
            __init_n(LENGTH);
        }



        //析构函数
        ~thread_safe_vector()
        {
            __destroy_mem(start_ , start_ + LENGTH);
            __deallocate_mem(start_, start_ + LENGTH);
        }
        //------------------------------------------------------------这一组是公共接口，对外的api----------------------------------------------
    public:
        // @parm ret通过这个引用来获取数组第一个元素的值，这里使用了移动拷贝来提高效率。
        // @返回bool值，如果为true代表获取了第一个元素应调用消费者，如果为false，当前数组为空，应调用生产者。
        bool pop_front(value_type _ret)
        {
            int nowCount = readableCount_--;
            if(nowCount < 0)//如果发现可以写的数量太少，就加回去，然后继续等待。这里建议失败后能够唤醒生产者。
            {
                ++readableCount_;
                return false;
            }
            
            node_type node;
            int idx = 0;
            //获取不成功就轮询
            while (node.nodeType_ != readable_vector_node)
            {
                idx = front_++;
                idx %= LENGTH;
                //front_ = front_ % LENGTH;//这里不处理坐等front溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码,解决了一个难题                
                node = std::atomic_exchange(start_ + idx , node);
            }
              _ret = LT::move(node.data_);
              std::atomic_exchange(start_ + idx, node_type());
               ++writableCount_;
            return true;
        }

        // 无参版本，会阻塞线程  
        value_type pop_front_choke()
        {
            while (true)
            {
                int nowCount = readableCount_--;
                if (nowCount < 0)//持续等待生产者生产
                {
                    ++readableCount_;
                }
                else { break; }
            }
            int idx = 0;
            node_type node(forbbiden_vector_node);
            //获取不成功就轮询
            while (node.nodeType_ != readable_vector_node)
            {            
                idx = front_++;
                idx %= LENGTH;
                //front_ = front_ % LENGTH;//这里不处理坐等front溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码,解决了一个难题               
                node = std::atomic_exchange(start_ + idx, node);
            }            
            std::atomic_exchange(start_ + idx, node_type());
            ++writableCount_;
            if (node.data_ > 200 || node.data_ < 0) {
                int i = 1;
            }
            return node.data_
        }

        // @parm _args 可以传入左值或者右值引用
        // @返回bool值，如果为true代表填充了一个元素应调用生产者，如果为false，当前数组为空，应调用生产者。
        bool push_back(value_type && _value) {

            int nowCount = writableCount_--;
            if(nowCount < 0)
            {
                ++writableCount_;
                return false;
            }

            int idx = 0;
            node_type node(forbbiden_vector_node);
            //获取不成功就轮询
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//以防万一
                //back_ = back_ % LENGTH;//这里不处理坐等back溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码
                node = std::atomic_exchange(start_ + idx, node);
            }
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node,LT::move(_value)));
            ++readableCount_;
            return true;
        }

        // 无返回值版本，会阻塞线程等待消费者消费
        void push_back_choke(value_type&& _value) {  
            while (true)
            {
                int nowCount = writableCount_--;
                if (nowCount < 0)
                {
                    ++writableCount_;               
                }
                else {break;}
            }
            
            int idx = 0;
            node_type node(forbbiden_vector_node, _value);
            //获取不成功就轮询
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//以防万一
                //back_ = back_ % LENGTH;//这里不处理坐等back溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码
                node = std::atomic_exchange(start_ + idx, node);
            }           
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            //支持右值或左值
            ++readableCount_;
        }

        bool push_back(const_reference _value) {

            int nowCount = writableCount_--;
            if (nowCount < 0)
            {
                ++writableCount_;
                return false;
            }

            int idx = 0;
            node_type node(forbbiden_vector_node);
            //获取不成功就轮询
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//以防万一
                //back_ = back_ % LENGTH;//这里不处理坐等back溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码
                node = std::atomic_exchange(start_ + idx, node);
            }
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            ++readableCount_;
            return true;
        }

        // 无返回值版本，会阻塞线程等待消费者消费
        void push_back_choke(const_reference _value) {
            
            while (true)
            {
                int nowCount = writableCount_--;
                if (nowCount < 0)
                {
                    ++writableCount_;
                }
                else { break; }
            }

            int idx = 0;
            node_type node(forbbiden_vector_node, _value);
            //获取不成功就轮询
            while (node.nodeType_ != writable_vector_node)
            {              
                idx = back_++;
                idx %= LENGTH;//以防万一
                //back_ = back_ % LENGTH;//这里不处理坐等back溢出然后回到0，变成一个循环，减少了一步运算和一个临界区代码
                node = std::atomic_exchange(start_ + idx, node);
                
            }           
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            //支持右值或左值
            ++readableCount_;
        }

        bool full()
        {
            return !writableCount_;
        }

        bool  empty()
        {
            return !readableCount_;
        }

   
        //--------------------------------------------------------------容量配置器相关-----------------------------------------
        Alloc get_allocator() const { return allocator_type(); }
        /////********************************************************************************************************
        //**************************************************内部实现**************************************************
        ////**********************************************************************************************************
    private:
        //----------------------------------------------声明一组函数用来配置空间，包括进行初始化------------------------
        //要完成以下内容:
        //1.配置一块大小为n的内存，并对齐进行初始化
        //2.配置一块大小为n的内存，对其中大小为m的部分进行拷贝初始化，剩下的内存进行
        //3.鉴于此，分为两部分，一部分是获得给定大小的内存，一部分是初始化给定大小的内存区域

        //空间配置函数
        typedef Alloc            allocator_type;

        //获得给定大小的内存区域,在这里有异常保证
        atomic_node_pointer __get_mem(size_type _size)
        {
            //事实上allo函数返回的是T*。但是不保证该内存大小一定为_size，
            try {
                atomic_node_pointer memPtr = allocator_type::allocate(_size);
                return static_cast<atomic_node_pointer>(memPtr);//vector实现时将T*定义成了atomic_node_pointer。
            }
            catch (...) {//这一层异常保证也许可以省略，因为在alloctor文件里面应该是提供了异常保证的
                return static_cast<atomic_node_pointer>(nullptr);
            }

        }

        //可进行右值初始化的函数
        template<class ...Args>
        void __construct_one(atomic_node_pointer _pos, Args&&... _args)
        {
            LT::construct(LT::address_of(*_pos), LT::forward<Args>(_args)...);
        }
        //初始化给定大小的区域。进行异常保证，这是左值构造
        void __construct_mem_n(atomic_node_pointer _begin, size_type _size, const_atomic_node_reference _value) {
            LT::uninitialized_fill_n(_begin, _size, _value);
        }
        //这里要保证_begin的大小足够，顺序复制
        template<class InputIter>
        void __construct_mem_iter(atomic_node_pointer _begin, InputIter _itBeg, InputIter _itEnd)
        {
            size_type n = LT::distance(_itBeg, _itEnd);
            LT::uninitialized_copy(_itBeg, _itEnd, _begin);
        }

        //析构对象的函数
        void __destroy_mem(atomic_node_pointer _first, atomic_node_pointer _last)
        {
            //下一层construct函数会自己进行类型萃取，识别是否需要调用析构函数
            LT::destroy(_first, _last);
        }
        //释放内存
        template<class atomic_node_pointer>
        void __deallocate_mem(atomic_node_pointer _first, atomic_node_pointer _endOfStorage)
        {
            allocator_type::deallocate(_first, static_cast<size_type>((_endOfStorage - start_)));
        }


        //-------------------------------------------初始化函数----------------------------------------------------------------------
        //进行一个给定初始Length的初始化。
        void __init_n(size_type _size)
        {
            atomic_node_pointer newMem = __get_mem(_size);
            for (int i = 0; i < _size; ++i)
            {
                LT::construct(newMem + i);
            }
            start_ = newMem;
            front_ = 0;
            back_ = 1;
            writableCount_ = LENGTH * 0.9; //注意，如果这里把可写的长度设为LENGTH，会出现死循环问题，原理目前还没想通
            readableCount_ = 0;
        }
    };
}