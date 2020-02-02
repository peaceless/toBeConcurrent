#include "event.h"
Event::~Event()
{}
Acceptor::Acceptor(int fd, int listenLimit_, std::shared_ptr<ThreadSafeQueue<Event>> th)
    : Event(fd), listenLimit(listenLimit_), events_queue(th) {
}
void Acceptor::init(std::string address, int listen_port)
{
//    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    struct sockaddr_in server_address;
//
//    bzero(&server_address, sizeof(server_address));
//    server_address.sin_family = AF_INET;
//    server_address.sin_addr.s_addr = inet_addr(address.c_str());
//    server_address.sin_port = htons(listen_port);

//    bind(sockfd, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address));
    listen(sockfd, listenLimit);
}

void Acceptor::handle()
{
    socklen_t clntAddrSize;
    sockaddr_in clntAddr;
    clntAddrSize = sizeof(clntAddr);
    int clientSocketFd;
    while ((clientSocketFd
        = accept(sockfd, reinterpret_cast<struct sockaddr *>(&clntAddr), &clntAddrSize)) > 0) {
            std::cout << "get here" << std::endl;
            Event event(clientSocketFd);
            std::cout << event.sockfd << "++" << std::endl;
            events_queue->push(event);
        }
}

Acceptor::~Acceptor()
{
    events_queue = nullptr;
}