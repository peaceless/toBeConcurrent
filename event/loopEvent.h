#ifndef LOOPEVENT_H_
#define LOOPEVENT_H_
#include <vector>
#include <memory>
#include <atomic>
#include "../event/event.h"
#include "../thread/threadpool.h"
#include <sys/epoll.h>

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
    std::vector<pEvent> events_queue;
    std::queue<int> error_queue;

    std::mutex mut;
};

class pEvent : public Event
{
public:
    void setNonBlock();
    void handle() override;
    pEvent(int fd, const loopEvent *le_);

private:
    loopEvent *le;
};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp)
    : epfd(fd), fd_queue(th), Event(fd), pool(tp)
{
    events_queue.emplace_back(pEvent(0, this));
    for (int i = 1; i < LIMIT; i++)
    {
        events_queue.emplace_back(pEvent(-1, this));
    }
}
void loopEvent::addErrorEvent(int fd)
{
    std::lock_guard<std::mutex> lk(mut);
    error_queue.push(fd);
}

#define LIMIT_PER_TIME 10
void loopEvent::handle()
{
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

            if (events_queue[*event_fd].sockfd != -1)
            {
                lg << "-1 error." << std::endl;
                for (int i = 0; i < 50; i++)
                {
                    lg << events_queue[i].sockfd << " ";
                }
                exit(-1);
            }
            lg << *event_fd << " ++" << std::endl;
            events_queue[*event_fd] = pEvent(*event_fd, this);
            events_queue[*event_fd].setNonBlock();
            events_queue[0].sockfd++;
        }

        int eventCount = epoll_wait(epfd, events, LIMIT, LIMIT);
        lg << eventCount << "events occured." << std::endl;

        for (int i = 0; i < eventCount; ++i)
        {
            if (events[i].events == (EPOLLERR | EPOLLHUP))
            {
                addErrorEvent(events[i].data.fd);
                lg << "ADD errorEvent " << events[i].data.fd << std::endl;
            }
            else if (events[i].events == EPOLLIN)
            {
                // if (events_queue[events[i].data.fd].flag == true)
                // continue;
                pool->enqueue(&pEvent::handle, &(events_queue[events[i].data.fd]));
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
    lg << "now how many links are in queue: " << events_queue[0].sockfd << std::endl;
    int p = 0;
    for (int i = 1; i < 50; i++)
        if (events_queue[i].sockfd != -1)
            p++;

    if (events_queue[0].sockfd != p)
    {
        lg << "error :" << events_queue[0].sockfd << " " << p << std::endl;
        for (int i = 0; i < 50; i++)
        {
            lg << events_queue[i].sockfd << " ";
        }
        lg << "\n"
           << std::endl;
        exit(-1);
    }
}

void loopEvent::removeEvent(int event_id)
{
    event.data.fd = events_queue[event_id].sockfd;
    events_queue[event_id].sockfd = -1;
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, event_id, &event))
    {
        lg << "...................delete error" << errno << std::endl;
    }
    close(event_id);
    events_queue[0].sockfd--;
    lg << "delete " << event_id << std::endl;
}

pEvent::pEvent(int fd, const loopEvent *le_)
    : Event(fd), le(const_cast<loopEvent *>(le_))
{
}

void pEvent::handle()
{
    char buffer[100] = "\0";
    while (1)
    {
        int i = recv(sockfd, buffer, 100, 0);
        if (i > 0) {
            lg << buffer << std::endl;
        }
        if (i == -1 && errno == EWOULDBLOCK)
            break;
        else if (i == 0)
        {
            le->addErrorEvent(sockfd);
            break;
        }
    }
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