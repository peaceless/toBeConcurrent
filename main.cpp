#include "objectasthreadarg.h"

int main()
{
    ObjectAsThreadArg x;
    std::cout << "----------" << std::endl;
    std::thread t(x);
    std::cout << "----------" << std::endl;
    t.join();
    return 0;
}
//#include <iostream>
//#include <sys/socket.h>
//#include <sys/epoll.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <errno.h>
//#include <string.h>
//
////using namespace  std;
//#define OPEN_MAX 100
//#define MAXLINE 5
//#define LISTENQ 20
//#define SERV_PORT 5000
//#define INFTIM 1000
//
//void setnonblocking(int sock)
//{
//    int opts;
//    opts = fcntl(sock,F_GETFL);
//    if (opts < 0) {
//        perror("fcntl(sock,GETFL)");
//        exit(1);
//    }
//    opts = opts | O_NONBLOCK;
//    if (fcntl(sock,F_SETFL,opts) < 0) {
//        perror("fcntl(sock,SETFL,opts");
//        exit(1);
//    }
//}
//
//int main(int argc,char * argv[])
//{
//    using std::cout;
//    using std::endl;
//    //fd is a handle to control event such as file or process etc.
//    int i,maxi,listenfd,connfd,sockfd,epfd,nfds,portsnumber;
//    ssize_t n;
//    char line[MAXLINE];
//    socklen_t clilen;
//
//    if (2 == argc) {
//        if ((portsnumber = atoi(argv[1])) < 0) {
//            fprintf(stderr,"Usage:%s portsnumber\n",argv[0]);
//            return 1;
//        }
//    } else {
//        fprintf(stderr,"Usage:%s portsnumber\n",argv[0]);
//        return 1;
//    }
//
//
//    //declare variable of epoll_event struct,ev registe event
//    //arrays to callback happened event
//    struct epoll_event ev,events[20];
//    //fd handle accpet
//    epfd = epoll_create(256);
//    struct sockaddr_in clientaddr;
//    struct sockaddr_in serveraddr;
//    listenfd = socket(AF_INET,SOCK_STREAM,0);
//    //set socket to unblocking
//    //setnonblocking(listenfd);
//
//    //set filedefination about events to deals
//    ev.data.fd = listenfd;
//    //set event type
//    //ET = edge trigger
//    // which means just callback once,next time will not awake
//    ev.events = EPOLLIN | EPOLLET;
//    //regist epoll event
//    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
//
//    bzero(&serveraddr,sizeof(serveraddr));
//
//    serveraddr.sin_family = AF_INET;
//    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
//    //char * local_addr = "127.0.0.1";
//    //inet_aton(local_addr,&(serveraddr.sin_addr));
//
//    serveraddr.sin_port = htons(static_cast<uint16_t>(portsnumber));
//    bind(listenfd,(sockaddr *)&serveraddr,sizeof(serveraddr));
//    listen(listenfd,LISTENQ);
//    maxi = 0;
//    for(;;) {
//        //wait epoll event
//        //epoll_wait:
//        //first argument: fd which epoll_create return
//        //second argument : the events set kernel returned
//        // third : max events number
//        // forth : timeout second 0 is immiditaly -1 is not sure
//        nfds = epoll_wait(epfd,events,20,500);
//
//        //deal
//        for (i = 0;i < nfds; ++i) {
//            if (events[i].data.fd == listenfd) {
//                //if observer find a new link
//                clilen = sizeof (clientaddr);
//                connfd = accept(listenfd,(sockaddr *)&clientaddr,& clilen);
//                if (connfd < 0) {
//                    perror("connfd < 0");
//                    exit(1);
//                }
//                //setnoblocking(connfd)
//
//                char * str = inet_ntoa(clientaddr.sin_addr);
//                cout << "accept a connection from " << str << endl;
//
//                //set fd for read
//                ev.data.fd = connfd;
//                //set read event for registe
//                ev.events = EPOLLIN | EPOLLET;
//                //registe ev
//                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
//            } else if (events[i].events & EPOLLIN) {
//                //if link exist and get data in
//                cout << "EPOLLIN" << endl;
//                //why set file description to -1 instead of delete?
//                if ((sockfd = events[i].data.fd) < 0)
//                    continue;
//                if ((n = read(sockfd,line,MAXLINE)) < 0) {
//                    if (errno == ECONNRESET) {
//                        close(sockfd);
//                        events[i].data.fd = -1;
//                    } else
//                        cout << "readline error" << endl;
//                } else if (n == 0) {
//                    close(sockfd);
//                    events[i].data.fd = -1;
//                }
//                line[n] = '\0';
//                cout << "read" << line << endl;
//
//                //why set for write?
//                //set fd for write
//                ev.data.fd = sockfd;
//                //set write event for registe
//                ev.events = EPOLLOUT | EPOLLET;
//                //chang event on sockfd to EPOLLOUT
//                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
//            } else if (events[i].events & EPOLLOUT) {
//                //have data to send
//                sockfd = events[i].data.fd;
//                write(sockfd,line,n);
//                //set fd for read
//                ev.data.fd = sockfd;
//                //set read event for registe
//                ev.events = EPOLLIN | EPOLLET;
//                //change sockfd to deal EPOLIN
//                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
//            }
//        }
//    }
//    return 0;
//}
//
