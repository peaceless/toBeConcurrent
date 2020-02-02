#include <iostream>
#include <thread>
#include <stdlib.h>
#include <time.h>
#include "../event/event.h"
#include "../event/loopEvent.h"

int main(int argc, char* argv[])
{
    std::shared_ptr<ThreadSafeQueue<Event>> th = std::make_shared<ThreadSafeQueue<Event> >();
    std::string address = "127.0.0.1";
    int listen_port = 8080;
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_address;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address.c_str());
    server_address.sin_port = htons(listen_port);

    bind(sockfd, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address));
    Acceptor ac(sockfd, 500, th);
    ac.init("127.0.0.1", 8080);
    std::thread t2(&Acceptor::handle,ac);

    int epfd = epoll_create(500);
    loopEvent le(epfd, th);
    std::thread t1(le);
    t1.join();
    t2.join();

    return 0;
}
