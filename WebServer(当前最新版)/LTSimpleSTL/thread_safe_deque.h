//@Author Lin Tao
//@Email putaopu@qq.com

#pragma once
//这个头文件是一个配接器，用来对外提供线程安全的thread_safe_deque
//消费者和生产者的思路保证了线程安全。

#include <assert.h>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "deque.h"
#include "vector.h"
#include "string.h"


namespace LT {
    template<class T, class Sequence = std::deque<T>>
    class thread_safe_deque {

    public:
        //照例定义一些基本类型
        typedef typename Sequence::value_type						  value_type;
        typedef typename Sequence::reference						  reference;
        typedef typename Sequence::size_type						  size_type;
        typedef typename Sequence::const_reference                    const_reference;
        
    private:
        Sequence container_;
        size_type capacity_;
        std::mutex mtx_;
        bool isClose_;
        std::condition_variable condConsumer_;
        std::condition_variable condProducer_;

    public:
        explicit thread_safe_deque(size_type _maxCapacity = 1000)
        {             
           assert(_maxCapacity > 0);
           assert(_maxCapacity > 0);
            isClose_ = false;
        }

        ~thread_safe_deque()
        {
            close();
        }

        void close()
        {
            {
                std::lock_guard<std::mutex> locker(mtx_);
                container_.clear();
                isClose_ = true;
            }
            condProducer_.notify_all();
            condConsumer_.notify_all();
        }

        bool empty()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return container_.empty();
        }

        bool full()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return container_.size() >= capacity_;
        }

        void clear()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            container_.clear();
        }

        size_type size()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return container_.size();
        }

        size_type capacity()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return capacity_;
        }

        T front()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return container_.front();
        }

        T back()
        {
            std::lock_guard<std::mutex> locker(mtx_);
            return container_.back();
        }
       
        void push_back(const T& _item)
        {
            std::unique_lock<std::mutex> locker(mtx_);
            while (container_.size() >= capacity_) {
                condProducer_.wait(locker);
            }
            container_.push_back(_item);
            condConsumer_.notify_one();
        }

        void push_front(const T& _item)
        {
            std::unique_lock<std::mutex> locker(mtx_);
            while (container_.size() >= capacity_) {
                condProducer_.wait(locker);
            }
            container_.push_front(_item);
            condConsumer_.notify_one();
        }

        bool pop(T& _item)
        {
            std::unique_lock<std::mutex> locker(mtx_);
            while (container_.empty()) {
                condConsumer_.wait(locker);
                if (isClose_) {
                    return false;
                }
            }
            _item = container_.front();
            container_.pop_front();
            condProducer_.notify_one();
            return true;
        }

        bool pop(T& _item, int _timeout)
        {
            std::unique_lock<std::mutex> locker(mtx_);
            while (container_.empty()) {
                if (condConsumer_.wait_for(locker, std::chrono::seconds(_timeout))
                    == std::cv_status::_timeout) {
                    return false;
                }
                if (isClose_) {
                    return false;
                }
            }
            _item = container_.front();
            container_.pop_front();
            condProducer_.notify_one();
            return true;
        }

        void flush()
        {
            condConsumer_.notify_one();
        } 
    };
}

