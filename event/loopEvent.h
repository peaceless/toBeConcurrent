// handle a epoll queue
// deliver event to thread
// get event from acceptor
#ifndef LOOPEVENT_H_
#define LOOPEVENT_H_
#include "../event/event.h"
#include <sys/epoll.h>

class loopEvent : public Event{
public:
    loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<Event>> th);
    void operator()();

private:
    int epfd;
    epoll_event events[500];
    std::shared_ptr<ThreadSafeQueue<Event> > events_queue;
};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<Event>> th)
    : epfd(fd), events_queue(th), Event(fd)
{
}
#define LIMIT_PER_TIME 10
void loopEvent::operator()()
{
    // epfd = epoll_create(500);
    while(1) {
        // get new events from events_queue
        for (int i = 0; i < LIMIT_PER_TIME && events_queue->size(); i++)
        {
            std::shared_ptr<Event> event = events_queue->TryPop();
            epoll_ctl(epfd, EPOLL_CTL_ADD, event->sockfd, events);
        }
        int eventCount = epoll_wait(epfd, events, 500, 500);
        for(int i = 0; i < eventCount; ++i) {
            std::cout << events[i].data.fd << " catch event" << std::endl;
            // get a thread to handle this event
           if(events[i].events == EPOLLERR)
           {
               close(events[i].data.fd);
           } else {
               std::cout << events[i].events << " " << events[i].data.fd << std::endl;
           }
        }
    }
}
#endif