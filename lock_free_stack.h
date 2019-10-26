#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H

#include <memory>
template<typename T>
class lock_free_stack
{
  private:
    struct node
    {
        std::shared_ptr<T> data;
        std::shared_ptr<node> next;
        node(const T& data_) : data(std::make_shared<T>(data_))
        {}
    };
    std::shared_ptr<node> head;
public:
    void push(const T& data)
    {
        const std::shared_ptr<node> new_node = std::make_shared<node>(data);
        new_node->next = head;
        while(!std::atomic_compare_exchange_strong(&head, &new_node->next, new_node));
    }
    std::shared_ptr<T> pop()
    {
        std::shared_ptr<node> old_head = std::atomic_load(&head);
        while(old_head && !std::atomic_compare_exchange_weak(&head, &old_head, old_head->next));
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
};
#endif // LOCK_FREE_STACK_H
