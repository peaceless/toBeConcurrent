#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> // for bzero

#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

void setNonBlocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(sock, GETFL)");
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int i, maxi, listenfd, connfd, sockfd, epfd, nfds, portnum;
    ssize_t n;
    char line[MAXLINE];
    socklen_t clilen;

    if (2 == argc) {
        if ((portnum = atoi(argv[1])) < 0) {
            fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
            return 1;
        }
    } else {
        fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
        return 1;
    }

    // 声明epoll_event结构体变量，ev用于注册事件，数组用于回传待处理事件
    struct epoll_event ev, events[20];
    // 生成用以accept的文件描述符
    epfd = epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 把socket设置为非阻塞式
    setNonBlocking(listenfd);
    // 设置要处理的事件相关的描述符
    ev.data.fd = listenfd;
    // 设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;

    // 注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    char* local_addr = "127.0.0.1";
    inet_aton(local_addr, &(serveraddr.sin_addr));

    serveraddr.sin_port = htons(portnum);
    bind(listenfd, reinterpret_cast<sockaddr *>(&serveraddr), sizeof(serveraddr));
    listen(listenfd, LISTENQ);
    maxi = 0;
    while(true) {
        // 等待epoll事件的发生
        nfds = epoll_wait(epfd, events, 20, 500);

        // 处理发生的事件
        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, reinterpret_cast<sockaddr*>(&clientaddr), &clilen);
                if (connfd < 0) {
                    perror("connfd < 0");
                    exit(1);
                }
                setNonBlocking(connfd);
                char *str = inet_ntoa(clientaddr.sin_addr);
                std::cout << "accept a connection from " << str << std::endl;
                // 设置用于读操作的文件描述符
                ev.data.fd = connfd;
                // 设置用于注册的读操作事件
                ev.events = EPOLLIN | EPOLLET |EPOLLOUT;
                // 注册ev
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                std::cout << "EPOLLIN" << std::endl;
                if ((sockfd = events[i].data.fd) <0 )
                    continue;
                if ((n = read(sockfd, line, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        close(sockfd);
                        events[i].data.fd = -1;
                    } else
                        std::cout << "readline error " << std::endl;
                } else if (n == 0) {
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                line[n] = '\0';
                std::cout << "read " << line << std::endl;
            } else if (events[i].events & EPOLLOUT) {
                sockfd = events[i].data.fd;
                write(sockfd, line, n);
            }
        }
    }
    return 0;
}
