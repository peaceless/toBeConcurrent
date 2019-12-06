#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> // bezero
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

class Event
{
public:
    Event() = delete;
    explicit Event(int fd);
    void handle();

private:
    int sockfd;
    void setNonBlock();
};

Event::Event(int fd) : sockfd(fd)
{
    setNonBlock();
}
void Event::setNonBlock()
{
    int opts;
    opts = fcntl(sockfd, F_GETFL);
    if (opts < 0) {
        perror("fcntl(sock, GETFL)");
        return;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        return ;
    }
}

void Event::handle()
{
    printf("hello,this is %d talking.\n", sockfd);
}