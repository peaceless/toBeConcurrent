#ifndef PACKAGED_TASK_H
#define PACKAGED_TASK_H

#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
std::mutex mutex;
std::deque<std::packaged_task<void()> > tasks;

bool gui_shutdown_msg_received();
void get_and_process_gui_msg();

void gui_thread()
{
    while(!gui_shutdown_msg_received())
    {
        get_and_process_gui_msg();
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(mutex);
            if (tasks.empty())
            {
                continue;
            }
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}
std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f);
    std::future<void> res = task.get_future();
    std::lock_guard<std::mutex> lk(mutex);
    tasks.push_back(std::move(task));
    return res;
}
#endif // PACKAGED_TASK_H
