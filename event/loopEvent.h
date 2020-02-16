// handle a epoll queue
// deliver event to thread
// get event from acceptor
#ifndef LOOPEVENT_H_
#define LOOPEVENT_H_
#include <vector>
#include "../event/event.h"
#include "../thread/threadpool.h"
#include <sys/epoll.h>

class loopEvent : public Event{
public:
    loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool* tp);
    void operator()();

private:
    enum  {LIMIT = 500};
    int epfd;
    ThreadPool* pool;
    epoll_event events[LIMIT], event;
    std::shared_ptr<ThreadSafeQueue<int> > fd_queue;
    std::vector<Event> events_queue;
};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool* tp)
    : epfd(fd), fd_queue(th), Event(fd), pool(tp)
{
    events_queue.reserve(LIMIT);
}
#define LIMIT_PER_TIME 10
void loopEvent::operator()()
{
    epfd = epoll_create(500);
    while(1) {
        for (int i = 0; i < LIMIT_PER_TIME && fd_queue->size(); i++)
        {
            std::shared_ptr<int> event_fd = fd_queue->TryPop();
            std::cout << "event_fd is " << *event_fd << std::endl;
            event.data.fd = *event_fd;
            event.events = EPOLLIN | EPOLLET;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, *event_fd, &event) == -1)
                std::cout << __func__ << " error: add event into epoll wrong!" << *event_fd << std::endl;
            events_queue.emplace_back(Event(*event_fd));
        }
        int eventCount = epoll_wait(epfd, events, 500, 500);
        for(int i = 0; i < eventCount; ++i) {
            std::cout << eventCount << "events occured." << std::endl;
            if(events[i].events == EPOLLERR)
            {
                std::cout << events[i].data.fd << "closed." << std::endl;
                close(events[i].data.fd);
            } else {
                pool->enqueue(&Event::handle, events_queue[events[i].data.fd]);
            }
        }
    }
}

bool swap(int i, std::vector<Event> vector, int std)
{
    for (int t = vector.size(); t > 0; t--)
    {
        if (vector[t].sockfd != std)
        {
            Event temp = vector[t];
            vector[t] = vector[i];
            vector[i] = temp;
        }
    }
    return true;
}
#endif