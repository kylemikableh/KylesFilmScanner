#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "RGBImage.h"

template <typename T>
class RGBImageQueue
{
    public:
        void push(T* value)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(value);
            cond_var_.notify_one();
        }

        bool pop(T*& value)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_var_.wait(lock, [this] { return !queue_.empty(); });
            value = queue_.front();
            queue_.pop();
            return true;
        }

        bool empty()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

    private:
        std::queue<T*> queue_;
        std::mutex mutex_;
        std::condition_variable cond_var_;
};
