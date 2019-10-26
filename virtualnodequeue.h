#ifndef VIRTUALNODEQUEUE_H
#define VIRTUALNODEQUEUE_H

#include <condition_variable>
#include <mutex>
#include <memory>
template<typename T>
class queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node* tail;

public:
    queue() : head(new node), tail(head.get())
    {}
    queue(const queue& other) = delete;
    queue& operator=(const queue& other) = delete;

    std::shared_ptr<T> try_pop()
    {
        if (head.get() == tail)
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }

    void push(T new_value)
    {
        std::shared_ptr<T> new_data(
                    std::make_shared(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};
template<typename T>
class threadsafe_queue
{
private:
  struct node
  {
      std::shared_ptr<T> data;
      std::unique_ptr<node> next;
  };
  std::mutex head_mutex;
  std::unique_ptr<node> head;
  std::mutex tail_mutex;
  node* tail;

  node* get_tail()
  {
      std::lock_guard<std::mutex> lk(tail_mutex);
      return tail;
  }
  std::unique_ptr<node> pop_head()
  {
      std::lock_guard<std::mutex> lk(head_mutex);
      if (head.get() == get_tail())
          return nullptr;
      std::unique_ptr<node> old_head = std::move(head);
      head = std::move(old_head->next);
      return old_head;
  }
public:
  threadsafe_queue():head(new node), tail(head.get())
  {}
  threadsafe_queue(const threadsafe_queue& other) = delete;
  threadsafe_queue& operator=(const threadsafe_queue&) = delete;

  std::shared_ptr<T> try_pop()
  {
      std::unique_ptr<node> old_head = pop_head();
      return old_head?old_head->data:std::shared_ptr<T>();
  }
  void push(T new_value)
  {
      std::shared_ptr<T> new_data(std::make_shared(std::move(new_value)));
      std::unique_ptr<node> p(new node);
      node* const new_tail = p.get();
      std::lock_guard<std::mutex> lk(tail_mutex);
      tail->data = new_data;
      tail->next = std::move(p);
      tail = new_tail;
  }
};

template<typename T>
class threadsafe_queue_
{
private :
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;

    node* get_tail()
    {
        std::lock_guard<std::mutex> lk(tail_mutex);
        return tail;
    }
    std::unique_ptr<node> pop_head()
    {
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> lk(head_mutex);
        data_cond.wait(lk,[&]{return head.get() != get_tail();});
        return lk;
    }
    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock<std::mutex> lk(wait_for_data());
        return pop_head();
    }
    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock<std::mutex> lk(wait_for_data());
        value = std::move(*head->data);
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard<std::mutex> lk(head_mutex);
        if (head.get() == get_tail())
            return std::unique_ptr<node>();
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head(T& value)
    {
        std::lock_guard<std::mutex> lk(head_mutex);
        if (head.get() == get_tail())
            return std::unique_ptr<node>();
        value = std::move(*head->data);
        return pop_head();
    }

public :
    threadsafe_queue_() : head(new node), tail(head.get())
    {}
    threadsafe_queue_(const threadsafe_queue_& other) = delete;
    threadsafe_queue_& operator=(const threadsafe_queue_& other) = delete;

    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
    bool try_pop(T& value)
    {
        std::unique_ptr<node> old_head = try_pop_head(value);
        return old_head;
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }
    void wait_and_pop(T& value)
    {
        std::unique_ptr<node> const old_head = wait_pop_head(value);
    }
    void push(T new_value);
    bool empty()
    {
        std::lock_guard<std::mutex> lk(head_mutex);
        return (head.get() == get_tail());
    }
};

template<typename T>
void threadsafe_queue_<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> lk(tail_mutex);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}
#endif // VIRTUALNODEQUEUE_H
