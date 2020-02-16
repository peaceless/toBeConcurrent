#ifndef EVENT_CPP_
#define EVENT_CPP_
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> // bezero
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>

#include "threadsafequeue.h"

class Event
{
public:
    Event() = delete;
    explicit Event(int fd);
    virtual void handle();
    virtual ~Event();

public:
    int sockfd;

private:
    void setNonBlock();
};

Event::Event(int fd) : sockfd(fd)
{
    // setNonBlock();
}
void Event::setNonBlock()
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

void Event::handle()
{
    // printf("hello,this is %d talking.\n", sockfd);
}

class Acceptor : public Event
{
public:
    explicit Acceptor(int fd, int listenLimit_, std::shared_ptr<ThreadSafeQueue<int>> th);
    void handle() override;
    void init(std::string address, int listen_port);
    ~Acceptor() override;

private:
    int listenLimit;
    std::shared_ptr<ThreadSafeQueue<int> > events_queue;
    struct sockaddr_in serv_addr;
};


#endif