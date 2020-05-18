#ifndef LOOPEVENT_H_
#define LOOPEVENT_H_
#include <vector>
#include <memory>
// #include <mutex>
#include <atomic>
#include "event.h"
#include "../thread/threadpool.h"
#include <sys/epoll.h>

#include "../httpUnit/HttpHandler.h"
class log
{
public:
    log();
    void out(const std::string &data);
    template <class T>
    std::ostream &operator<<(T data);
    std::mutex mut;
};
log::log()
{
}
template <class T>
std::ostream &log::operator<<(T data)
{
    std::lock_guard<std::mutex> lk(mut);
    return std::cout << data;
}

log lg;
class pEvent;
class loopEvent : public Event
{
public:
    loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp);
    void operator()();
    void addErrorEvent(int fd);
    void handle() override;

private:
    void cleanErrorEvent();
    void removeEvent(int event_id);

private:
    enum
    {
        LIMIT = 500
    };
    int epfd;
    ThreadPool *pool;
    epoll_event events[LIMIT], event;
    std::shared_ptr<ThreadSafeQueue<int>> fd_queue;
    std::vector<pEvent *> events_queue;
    std::queue<int> error_queue;

    std::mutex mut;
};

class pEvent : public Event
{
public:
    void setNonBlock();
    void handle() override;
    pEvent(int fd, const loopEvent *le_);

    std::mutex mut;
    int evcount;
private:
    loopEvent *le;
    HttpHandler handler;
};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp)
    : epfd(fd), fd_queue(th), Event(fd), pool(tp)
{
    events_queue.resize(LIMIT); // all place has a null value
    events_queue[0] = new pEvent(0, this);
}
void loopEvent::addErrorEvent(int fd)
{
    std::lock_guard<std::mutex> lk(mut);
    error_queue.push(fd);
}

#define LIMIT_PER_TIME 10
void loopEvent::handle()
{
    int max = 0;
    epfd = epoll_create(500);
    while (1)
    {
        for (int i = 0; i < LIMIT_PER_TIME; i++)
        {
            std::shared_ptr<int> event_fd = fd_queue->TryPop();
            if (!event_fd) // if queue is empty
                break;

            event.data.fd = *event_fd;
            event.events = EPOLLIN | EPOLLET;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, *event_fd, &event) == -1)
                lg << __func__ << " error: add event into epoll wrong!" << *event_fd << std::endl;

            if (events_queue[*event_fd] != nullptr)
            {
                lg << "-1 error." << std::endl;
                lg << *event_fd << "is exit." << std::endl;
                exit(-1);
            }
            events_queue[*event_fd] = new pEvent(*event_fd, this);
            events_queue[*event_fd]->setNonBlock();
            events_queue[0]->sockfd++;

            if(max < *event_fd) max = *event_fd;
        }

        int eventCount = epoll_wait(epfd, events, LIMIT, LIMIT);

        for (int i = 0; i < eventCount; ++i)
        {
            // XXX : epollerr & epollhup seem like only hanppen when server peer
            if (events[i].events == (EPOLLERR | EPOLLHUP))
            {
                exit(-1);
            } else if (events[i].events == EPOLLIN)
            {
                std::lock_guard<std::mutex> lk(events_queue[events[i].data.fd]->mut);
                if (events_queue[events[i].data.fd]->evcount > 0)
                {
                    events_queue[events[i].data.fd]->evcount ++;
                } else if (events_queue[events[i].data.fd]->evcount == 0) {
                    events_queue[events[i].data.fd]->evcount ++;
                    pool->enqueue(&pEvent::handle, events_queue[events[i].data.fd]);
                }
            }
        }
        cleanErrorEvent();
    }
}

void loopEvent::cleanErrorEvent()
{
    std::lock_guard<std::mutex> lk(mut);
    int t = error_queue.size();
    for (int i = 0; i < LIMIT && i < t; i++)
    {
        int p = error_queue.front();
        error_queue.pop();
        removeEvent(p);
    }
}

void loopEvent::removeEvent(int event_id)
{
    if (events_queue[event_id] == nullptr)
        return;
    // wait for all thread has handle over this sock
    while (events_queue[event_id]->evcount > 0)
        std::this_thread::yield;
    event.data.fd = events_queue[event_id]->sockfd;
    delete events_queue[event_id];
    events_queue[event_id] = nullptr;

    if (epoll_ctl(epfd, EPOLL_CTL_DEL, event_id, &event))
    {
        lg << "...................delete error" << errno << std::endl;
        exit(-1);
    }
    shutdown(event_id, SHUT_RDWR);
    close(event_id);
    events_queue[0]->sockfd--;
}

pEvent::pEvent(int fd, const loopEvent *le_)
    : Event(fd), le(const_cast<loopEvent *>(le_)), evcount(0), handler(fd)
{
}

void pEvent::handle()
{
    std::cout << "handle" << std::endl;
    char buffer[100] = "\0";
    bool flag = false;
    for (;;)
    {
        bzero(buffer, 100);
        int i = recv(sockfd, buffer, 100, 0);
        std::cout << i << "receive msg:" << buffer << std::endl;
        std::cout << "--------\n" << std::endl;
        if (i > 0) {
            handler.Handle(buffer);
            // httpParse.ParseData(buffer);
            // HttpRequest result;
            // result = httpParse.GetResult();
            // std::cout << "method is " << result.method << std::endl;
            // std::cout << "url is " << result.url << std::endl;
            // std::cout << "version is" << result.version << std::endl;
            // auto it = result.POST_Args.begin();
            // while (it != result.headers.end()){
                // std::cout << it->first << it->second << std::endl;
                // it ++;
            // }
            // std::cout << buffer;
            // specify how to deal event
        } else if (i == -1 && errno == EWOULDBLOCK) // read all data from buffer
        {
            std::lock_guard<std::mutex> lk(mut);
            evcount --;
            if (!evcount)
                break;
            if (evcount < 0)
            {
                lg << "count of handle has negitive" << std::endl;
                exit(-1);
            }
        } else if (i == 0)    // client side has closed connection
        {
            std::lock_guard<std::mutex> lk(mut);
            evcount = -2;   // handle over
            le->addErrorEvent(sockfd);
            break;
        }
    }
    std::cout << "handle over." << std::endl;
}

void pEvent::setNonBlock()
{
    int opts;
    opts = fcntl(sockfd, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock, GETFL)");
        return;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock, SETFL, opts)");
        return;
    }
}
#endif