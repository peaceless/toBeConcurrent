#ifndef LOOPEVENT_H_
#define LOOPEVENT_H_
#include <vector>
#include <memory>
#include "../event/event.h"
#include "../thread/threadpool.h"
#include <sys/epoll.h>

class loopEvent : public Event{
public:
    loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool* tp);
    void operator()();
    void addErrorEvent(int fd);
    void handle() override;

private:
    void cleanErrorEvent();
    void removeEvent(int event_id);

private:
    enum  {LIMIT = 500};
    int epfd;
    ThreadPool* pool;
    epoll_event events[LIMIT], event;
    std::shared_ptr<ThreadSafeQueue<int> > fd_queue;
    std::vector<Event> events_queue;
    // ThreadSafeQueue<int> error_queue;
    std::queue<int> error_queue;

    std::mutex mut;

};

//class pEvent : public Event
//{
//public:
//    void setNonBlock();
//    void handle() override;
//    pEvent(int fd, const loopEvent* le_);
//
//private:
//    loopEvent* le;
//};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool* tp)
    : epfd(fd), fd_queue(th), Event(fd), pool(tp)
{
    events_queue.emplace_back(Event(0));
    for (int i = 1; i < LIMIT; i++) {
        events_queue.emplace_back(Event(-1));
    }
}
void loopEvent::addErrorEvent(int fd)
{
    std::lock_guard<std::mutex> lk(mut);
    std::cout << __func__ << fd << "...." << std::endl;
    error_queue.push(fd);
}

#define LIMIT_PER_TIME 10
void loopEvent::handle()
{
    epfd = epoll_create(500);
    while(1) {
        for (int i = 0; i < LIMIT_PER_TIME && !fd_queue->IsEmpty(); i++)
        {
            std::shared_ptr<int> event_fd = fd_queue->TryPop();

            event.data.fd = *event_fd;
            event.events = EPOLLIN | EPOLLET;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, *event_fd, &event) == -1)
                std::cout << __func__ << " error: add event into epoll wrong!" << *event_fd << std::endl;

            events_queue[*event_fd] = Event(*event_fd);
            events_queue[*event_fd].setNonBlock();
            events_queue[0].sockfd ++;
        }

        int eventCount = epoll_wait(epfd, events, LIMIT, LIMIT);

        for(int i = 0; i < eventCount; ++i) {
            std::cout << eventCount << "events occured." << events[i].events << std::endl;
            if(events[i].events == (EPOLLERR | EPOLLHUP) )
            {

                addErrorEvent(events[i].data.fd);
                std::cout << "add errorEvent " << events[i].data.fd << std::endl;

            } else if (events[i].events == EPOLLIN){
                pool->enqueue(&Event::handle, events_queue[events[i].data.fd]);

                // reset events
                event.data.fd = events[i].data.fd;
                epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &event);
            }
        }
        cleanErrorEvent();
    }
}

void loopEvent::cleanErrorEvent()
{
    std::lock_guard<std::mutex> lk(mut);
    int t = error_queue.size();
    for(int i = 0; i < LIMIT && i < t; i++) {
        int p = error_queue.back();
        error_queue.pop();
        removeEvent(p);
    }
}

void loopEvent::removeEvent(int event_id)
{
    event.data.fd = events_queue[event_id].sockfd;
    events_queue[event_id].sockfd = -1;
    epoll_ctl(epfd, EPOLL_CTL_DEL, event_id, &event);
    close(event_id);
    events_queue[0].sockfd --;
}



//pEvent::pEvent(int fd, const loopEvent* le_)
//    : Event(fd), le(const_cast<loopEvent*>(le_))
//{}
//
//void pEvent::handle()
//{
//    char buffer[100] = "\0";
//    if (!recv(sockfd, buffer, 100, 0))
//    {
//        le->addErrorEvent(sockfd);
//    }
//}
//
//void pEvent::setNonBlock()
//{
//    int opts;
//    opts = fcntl(sockfd, F_GETFL);
//    if (opts < 0)
//    {
//        perror("fcntl(sock, GETFL)");
//        return;
//    }
//    opts = opts | O_NONBLOCK;
//    if (fcntl(sockfd, F_SETFL, opts) < 0)
//    {
//        perror("fcntl(sock, SETFL, opts)");
//        return;
//    }
//}
#endif