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

//���ļ�����atomicʵ����������ѭ������
//�����½ӿڽ����̰߳�ȫ��֤��
//Ԥ��ʼ����֧����ֵ���ú��ƶ������Ч�ʡ�
//ʹ���������ߺ�������ģ�ͣ�ͨ������ֵ����ʾӦ�õ��������߻���������
//push_back()
//pop_front()
//�������Ϻ������������̵߳İ汾, ȷ��һ���ɹ�

//�Ż����ܣ������������ִ�Ҫ��������ôһ�������ܣ���ô���LEGNTH�������ó�2���ݣ�������������λ��������ȡ�ࡣ

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
        //�ȶ���һЩ����
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

        //����vector�ڲ���������Ҫ��ָ��
    private:
        atomic_node_pointer start_;
        std::atomic<size_type> front_; //������ʶѭ�������ͷ
        std::atomic<size_type> back_;  //������ʶѭ�������β
        std::atomic<int> writableCount_; //����ȷ����������Ƿ�Ϊ�գ�����Ϊ��
        std::atomic<int> readableCount_;


/////********************************************************************************************************
//**************************************************����ӿ�**************************************************
////**********************************************************************************************************
    public:
        //дһЩ���캯��
         thread_safe_vector()
             :start_(0)
        {
            //��ÿ��Ԫ�ؽ���Ԥ��ʼ��
            __init_n(LENGTH);
        }



        //��������
        ~thread_safe_vector()
        {
            __destroy_mem(start_ , start_ + LENGTH);
            __deallocate_mem(start_, start_ + LENGTH);
        }
        //------------------------------------------------------------��һ���ǹ����ӿڣ������api----------------------------------------------
    public:
        // @parm retͨ�������������ȡ�����һ��Ԫ�ص�ֵ������ʹ�����ƶ����������Ч�ʡ�
        // @����boolֵ�����Ϊtrue�����ȡ�˵�һ��Ԫ��Ӧ���������ߣ����Ϊfalse����ǰ����Ϊ�գ�Ӧ���������ߡ�
        bool pop_front(value_type _ret)
        {
            int nowCount = readableCount_--;
            if(nowCount < 0)//������ֿ���д������̫�٣��ͼӻ�ȥ��Ȼ������ȴ������ｨ��ʧ�ܺ��ܹ����������ߡ�
            {
                ++readableCount_;
                return false;
            }
            
            node_type node;
            int idx = 0;
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != readable_vector_node)
            {
                idx = front_++;
                idx %= LENGTH;
                //front_ = front_ % LENGTH;//���ﲻ��������front���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������,�����һ������                
                node = std::atomic_exchange(start_ + idx , node);
            }
              _ret = LT::move(node.data_);
              std::atomic_exchange(start_ + idx, node_type());
               ++writableCount_;
            return true;
        }

        // �޲ΰ汾���������߳�  
        value_type pop_front_choke()
        {
            while (true)
            {
                int nowCount = readableCount_--;
                if (nowCount < 0)//�����ȴ�����������
                {
                    ++readableCount_;
                }
                else { break; }
            }
            int idx = 0;
            node_type node(forbbiden_vector_node);
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != readable_vector_node)
            {            
                idx = front_++;
                idx %= LENGTH;
                //front_ = front_ % LENGTH;//���ﲻ��������front���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������,�����һ������               
                node = std::atomic_exchange(start_ + idx, node);
            }            
            std::atomic_exchange(start_ + idx, node_type());
            ++writableCount_;
            if (node.data_ > 200 || node.data_ < 0) {
                int i = 1;
            }
            return node.data_
        }

        // @parm _args ���Դ�����ֵ������ֵ����
        // @����boolֵ�����Ϊtrue���������һ��Ԫ��Ӧ���������ߣ����Ϊfalse����ǰ����Ϊ�գ�Ӧ���������ߡ�
        bool push_back(value_type && _value) {

            int nowCount = writableCount_--;
            if(nowCount < 0)
            {
                ++writableCount_;
                return false;
            }

            int idx = 0;
            node_type node(forbbiden_vector_node);
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//�Է���һ
                //back_ = back_ % LENGTH;//���ﲻ��������back���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������
                node = std::atomic_exchange(start_ + idx, node);
            }
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node,LT::move(_value)));
            ++readableCount_;
            return true;
        }

        // �޷���ֵ�汾���������̵߳ȴ�����������
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
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//�Է���һ
                //back_ = back_ % LENGTH;//���ﲻ��������back���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������
                node = std::atomic_exchange(start_ + idx, node);
            }           
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            //֧����ֵ����ֵ
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
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != writable_vector_node)
            {
                idx = back_++;
                idx %= LENGTH;//�Է���һ
                //back_ = back_ % LENGTH;//���ﲻ��������back���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������
                node = std::atomic_exchange(start_ + idx, node);
            }
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            ++readableCount_;
            return true;
        }

        // �޷���ֵ�汾���������̵߳ȴ�����������
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
            //��ȡ���ɹ�����ѯ
            while (node.nodeType_ != writable_vector_node)
            {              
                idx = back_++;
                idx %= LENGTH;//�Է���һ
                //back_ = back_ % LENGTH;//���ﲻ��������back���Ȼ��ص�0�����һ��ѭ����������һ�������һ���ٽ�������
                node = std::atomic_exchange(start_ + idx, node);
                
            }           
            std::atomic_exchange(start_ + idx, node_type(readable_vector_node, LT::move(_value)));
            //֧����ֵ����ֵ
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

   
        //--------------------------------------------------------------�������������-----------------------------------------
        Alloc get_allocator() const { return allocator_type(); }
        /////********************************************************************************************************
        //**************************************************�ڲ�ʵ��**************************************************
        ////**********************************************************************************************************
    private:
        //----------------------------------------------����һ�麯���������ÿռ䣬�������г�ʼ��------------------------
        //Ҫ�����������:
        //1.����һ���СΪn���ڴ棬��������г�ʼ��
        //2.����һ���СΪn���ڴ棬�����д�СΪm�Ĳ��ֽ��п�����ʼ����ʣ�µ��ڴ����
        //3.���ڴˣ���Ϊ�����֣�һ�����ǻ�ø�����С���ڴ棬һ�����ǳ�ʼ��������С���ڴ�����

        //�ռ����ú���
        typedef Alloc            allocator_type;

        //��ø�����С���ڴ�����,���������쳣��֤
        atomic_node_pointer __get_mem(size_type _size)
        {
            //��ʵ��allo�������ص���T*�����ǲ���֤���ڴ��Сһ��Ϊ_size��
            try {
                atomic_node_pointer memPtr = allocator_type::allocate(_size);
                return static_cast<atomic_node_pointer>(memPtr);//vectorʵ��ʱ��T*�������atomic_node_pointer��
            }
            catch (...) {//��һ���쳣��֤Ҳ�����ʡ�ԣ���Ϊ��alloctor�ļ�����Ӧ�����ṩ���쳣��֤��
                return static_cast<atomic_node_pointer>(nullptr);
            }

        }

        //�ɽ�����ֵ��ʼ���ĺ���
        template<class ...Args>
        void __construct_one(atomic_node_pointer _pos, Args&&... _args)
        {
            LT::construct(LT::address_of(*_pos), LT::forward<Args>(_args)...);
        }
        //��ʼ��������С�����򡣽����쳣��֤��������ֵ����
        void __construct_mem_n(atomic_node_pointer _begin, size_type _size, const_atomic_node_reference _value) {
            LT::uninitialized_fill_n(_begin, _size, _value);
        }
        //����Ҫ��֤_begin�Ĵ�С�㹻��˳����
        template<class InputIter>
        void __construct_mem_iter(atomic_node_pointer _begin, InputIter _itBeg, InputIter _itEnd)
        {
            size_type n = LT::distance(_itBeg, _itEnd);
            LT::uninitialized_copy(_itBeg, _itEnd, _begin);
        }

        //��������ĺ���
        void __destroy_mem(atomic_node_pointer _first, atomic_node_pointer _last)
        {
            //��һ��construct�������Լ�����������ȡ��ʶ���Ƿ���Ҫ������������
            LT::destroy(_first, _last);
        }
        //�ͷ��ڴ�
        template<class atomic_node_pointer>
        void __deallocate_mem(atomic_node_pointer _first, atomic_node_pointer _endOfStorage)
        {
            allocator_type::deallocate(_first, static_cast<size_type>((_endOfStorage - start_)));
        }


        //-------------------------------------------��ʼ������----------------------------------------------------------------------
        //����һ��������ʼLength�ĳ�ʼ����
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
            writableCount_ = LENGTH * 0.9; //ע�⣬�������ѿ�д�ĳ�����ΪLENGTH���������ѭ�����⣬ԭ��Ŀǰ��û��ͨ
            readableCount_ = 0;
        }
    };
}