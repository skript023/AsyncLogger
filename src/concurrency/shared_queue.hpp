#pragma once
#include <queue>
#include <mutex>
#include "mutex.hpp"
#include <exception>
#include "conditional_variable.hpp"

/**
 * from Kjellkod/concurrent on Github
 * 
 */
namespace al
{
    template<typename T>
    class shared_queue
    {
    public:
        shared_queue() = default;

        void push(T item)
        {
            {
                std::lock_guard<Mutex> lock(m_);
                queue_.push(std::move(item));
            }
            data_cond_.notify_one();
        }

        /// \return immediately, with true if successful retrieval
        bool try_and_pop(T& popped_item)
        {
            std::lock_guard<Mutex> lock(m_);
            if (queue_.empty()) {
                return false;
            }
            popped_item = std::move(queue_.front());
            queue_.pop();
            return true;
        }

        /// Try to retrieve, if no items, wait till an item is available and try again
        void wait_and_pop(T& popped_item)
        {
            std::unique_lock<Mutex> lock(m_);
            while (queue_.empty()) {
                data_cond_.wait(lock);
                //  This 'while' loop is equal to
                //  data_cond_.wait(lock, [](bool result){return !queue_.empty();});
            }
            popped_item = std::move(queue_.front());
            queue_.pop();
        }

        bool empty() const
        {
            std::lock_guard<Mutex> lock(m_);
            return queue_.empty();
        }

        size_t size() const
        {
            std::lock_guard<Mutex> lock(m_);
            return queue_.size();
        }

    private:
        std::queue<T> queue_;
        mutable Mutex m_;
        ConditionVariable data_cond_;

        shared_queue& operator=(const shared_queue&) = delete;
        shared_queue(const shared_queue& other) = delete;
        
    };
}