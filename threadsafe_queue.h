// a threadsafe queue implemation
#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
template<typename T>
class threadsafe_queue
{
public :
    threadsafe_queue(){}
    threadsafe_queue(const threadsafe_queue &other)
    {
        std::lock_guard<std::mutex> lk(other.mutex);
        data_queue = other.data_queue;
    }
    threadsafe_queue& operator=(const threadsafe_queue &) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        if (data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mutex);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mutex);
        data_cond.wait(lk,[this]{
            return !data_queue.empty();
        });
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mutex);
        data_cond.wait(lk,[this]{
            return !data_queue.empty();
        });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mutex);
        return data_queue.empty();
    }

private :
    mutable std::mutex mutex;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
};

#endif // THREADSAFE_QUEUE_H
