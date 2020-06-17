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
#include "mytimer.h"
#include "logger.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

class pEvent;
class loopEvent : public Event
{
public:
    loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp);
    ~loopEvent();
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

    TimeWheel timeCount;
    std::mutex mut;

    SSL_CTX *ctx;
};

class pEvent : public Event
{
public:
    void setNonBlock();
    void handle() override;
    pEvent(int fd, const loopEvent *le_, SSL_CTX * ctx);
    ~pEvent();

    std::mutex mut;
    int evcount;
private:
    loopEvent *le;
    HttpHandler handler;

    SSL *ssl;
};

loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp)
    : epfd(fd), fd_queue(th), Event(fd), pool(tp), timeCount(this)
{
    // load ssl module
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    events_queue.resize(LIMIT); // all place has a null value
    events_queue[0] = new pEvent(0, this, ctx);
}
loopEvent::~loopEvent()
{
    close(epfd);
    SSL_CTX_free(ctx);
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
                LOG_ERROR("add event into epoll wrong.", *event_fd);

            if (events_queue[*event_fd] != nullptr)
            {
                LOG_ERROR(*event_fd, "is exist.-1 error");
                exit(-1);
            }
            events_queue[*event_fd] = new pEvent(*event_fd, this, ctx);
            events_queue[*event_fd]->setNonBlock();
            events_queue[0]->sockfd++;

            if(max < *event_fd) max = *event_fd;
        }

        int eventCount = epoll_wait(epfd, events, LIMIT, LIMIT);

        timeCount.Timer(events, eventCount);
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
        timeCount.Remove(p);
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
        LOG_ERROR("delete erro.");
        exit(-1);
    }
    shutdown(event_id, SHUT_RDWR);
    close(event_id);
    events_queue[0]->sockfd--;
}

pEvent::pEvent(int fd, const loopEvent *le_, SSL_CTX *ctx)
    : Event(fd), le(const_cast<loopEvent *>(le_)), evcount(0), handler(fd)
{
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    if (SSL_accept(ssl) == -1) {
        LOG_ERROR("create ssl link failed.", fd);
        close(fd);
        exit(-1);
    }
}

void pEvent::handle()
{
    LOG_INFO("handle");
    char buffer[1024] = "\0";
    int buffer_size = sizeof(buffer);
    bool flag = false;
    for (;;)
    {
        bzero(buffer, buffer_size);
        int i = SSL_read(ssl, buffer, buffer_size);
        if (i > 0) {
            handler.Handle(buffer);
        } else if (i == -1 && errno == EWOULDBLOCK) // read all data from buffer
        {
            std::lock_guard<std::mutex> lk(mut);
            evcount --;
            if (!evcount)
                break;
            if (evcount < 0)
            {
                LOG_ERROR("count fo handle has been negitive.");
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
    LOG_INFO("handle over.");
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

pEvent::~pEvent()
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}
TimeWheel::TimeWheel(loopEvent *le)
    : le(le), tik(0)
{
    LOG_INFO("TimeWheel builded.");
    insertFlag = false;
    conns.resize(500, -1);
    wheel.resize(8);
}
TimeWheel::~TimeWheel()
{
}
void TimeWheel::Remove(int sock)
{
    int pos = conns[sock];
    if (pos == -1) {
        LOG_ERROR("delete a sock not exist.");
        return ;
    }
    wheel[pos].erase(sock);
    conns[sock] = -1;
}
void TimeWheel::Timer(epoll_event epollEvents[], int eventCount)
{
    if (insertFlag) {
        tik ++;
        tik = tik % 8;
    }
    int sock;
    for (int i = 0; i < eventCount; i++) {
        sock = epollEvents[i].data.fd;
        int tail = (tik + 7)%8;
        if (conns[sock] != -1) {
            int t = conns[sock];
            wheel[t].erase(sock);
            LOG_INFO("update timeout ", sock);
        }
        wheel[tail].insert(sock);
        conns[sock] = tail;
    }
    if (insertFlag) {
        for (int i : wheel[tik]) {
            LOG_INFO("remove event ", i);
            le->addErrorEvent(i);
        }
    }
    insertFlag = !insertFlag;
}
#endif