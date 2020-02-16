#include "event.h"
Event::~Event()
{}
Acceptor::Acceptor(int fd, int listenLimit_, std::shared_ptr<ThreadSafeQueue<int>> th)
    : Event(fd), listenLimit(listenLimit_), events_queue(th) {
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8080);

    bind(sockfd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr));
    listen(sockfd, listenLimit);
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
    // listen(sockfd, listenLimit);
}

void Acceptor::handle()
{
    socklen_t clntAddrSize;
    sockaddr_in clntAddr;
    clntAddrSize = sizeof(clntAddr);
    bzero(&clntAddr, clntAddrSize);
    int clientSocketFd;
    // std::cout << sockfd << "ready to connected..." << std::endl;
    while ((clientSocketFd
        = accept(sockfd, reinterpret_cast<struct sockaddr *>(&clntAddr), &clntAddrSize)) > 0) {
            // std::cout << __func__ << __LINE__ << std::endl;
            events_queue->push(clientSocketFd);
            std::cout << "clientSocketFd ++" << std::endl;
        }
    std::cout << __func__ << clientSocketFd << "end." << std::endl;
    system("pause");
}

Acceptor::~Acceptor()
{
    events_queue = nullptr;
}