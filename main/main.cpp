#include <iostream>
#include <thread>
#include <stdlib.h>
#include <time.h>
#include "../thread/threadpool.h"
#include "../event/event.h"
#include "../event/loopEvent.h"
<<<<<<< HEAD
//#include "../include/openssl/ssl.h"
#include "openssl/ssl.h"
#include <openssl/err.h>
=======
#include "../event/logger.hpp"

>>>>>>> master
int main(int argc, char* argv[])
{
    LOG_INFO("Server start");
    LOG_DORECORD();
    std::string address = "127.0.0.1";
    std::shared_ptr<ThreadSafeQueue<int>> th = std::make_shared<ThreadSafeQueue<int> >();
    int listen_port = 8080;
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_address;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address.c_str());
    server_address.sin_port = htons(listen_port);

    ThreadPool* tp = new ThreadPool(5);
    Acceptor ac(sockfd, 500, th);
    ac.init("127.0.0.1", 8080);
    std::thread t2(&Acceptor::handle, ac);

    int epfd = 0;//epoll_create(500);
    loopEvent le(epfd, th, tp);
    std::thread t1(&loopEvent::handle, &le);
    t1.join();
    t2.join();

    return 0;
}
