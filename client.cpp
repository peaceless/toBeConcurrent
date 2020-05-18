#include <string>
#include <cstdlib>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>
#include <vector>
#include <errno.h>

class Client
{
public:
    Client(std::string server_ip = "111.62.72.10");
    void operator()();
    //void connecting();

private:
    sockaddr_in serv_addr;
};

Client::Client(std::string server_ip)
{
    const char *ip = server_ip.c_str();
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(80);
}

void Client::operator()()
{
    while (1)
    {
        int sock;
        if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            std::cout << "Create socket fail!" << std::endl;
            exit(0);
        }
        std::cout << "create client " << sock << std::endl;
        if (connect(sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr))<0)
        {
            std::cout << "Can't connect!" << std::endl;
            std::cout << "errno is " << errno << std::endl;
            break;
            //  exit(0);
        }
        else
        {
            char remsg[100] = "\0";
            std::cout << sock << "++" << std::endl;
            write(sock, "GET /index.html HTTP/1.1\r\nHost: www.bilibili.com\r\n\r\n", 50);
            // recv(sock, remsg, 100, 0);
            std::cout << remsg << std::endl;
            sleep(2);
            close(sock);
            std::cout << sock << "--" << std::endl;
        }
    }
}

int main()
{
    Client my_client;
    std::vector<std::thread> vect;
    for(int i = 0; i < 1; i++)
    {
        vect.emplace_back(my_client);
    }
    // my_client();
    for(int i = 0; i < 1; i++)
        vect[i].join();
}
