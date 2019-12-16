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
    virtual void handle();
    virtual ~Event();

protected:
    void setFd(int fd) { sockfd = fd; }
    int getFd() { return sockfd; };

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
    printf("hello,this is %d talking.\n", sockfd);
}

class Acceptor : public Event
{
public:
    explicit Acceptor(int sockfd, int listenLimit);
    Acceptor() = delete;
    void handle() override;
    void init(std::string address, int listen_port);
    ~Acceptor() override;

private:
    int listenLimit;
};

Acceptor::Acceptor(int sockfd, int listenLimit_)
    : Event(sockfd), listenLimit(listenLimit_) {}
void Acceptor::init(std::string address, int listen_port)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setFd(fd);
    struct sockaddr_in server_address;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address.c_str());
    server_address.sin_port = htons(listen_port);

    bind(fd, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address));
    listen(fd, listenLimit);
}

void Acceptor::handle()
{
    socklen_t clntAddrSize;
    sockaddr_in clntAddr;
    clntAddrSize = sizeof(clntAddr);
    int clientSocketFd;
    while ((clientSocketFd
        = accept(getFd(), reinterpret_cast<struct sockaddr *>(&clntAddr), &clntAddrSize))) {
            auto event = new Event(clientSocketFd);
            // TODO: add event into workqueue;
        }
}