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
#include <ctime>

class Client
{
public:
    Client(std::string server_ip = "127.0.0.1");
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
    serv_addr.sin_port = htons(8080);
}

void Client::operator()()
{
    int i = 0;
    std::clock_t total_time;
    while (i++ < 10)
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
            // std::cout << sock << "++" << std::endl;
            std::clock_t start = clock();
            write(sock, "GET /index.html HTTP/1.1\r\n\r\n", 28);
            recv(sock, remsg, 100, 0);
            std::clock_t resumeTime = clock() - start;
            total_time += resumeTime;
            // while(recv(sock, remsg, 100, 0) > 0);
            sleep(1);
            close(sock);
            // std::cout << sock << "--" << std::endl;
        }
    }
    double tt = double(total_time) / CLOCKS_PER_SEC;
    tt /= 10;
    std::cout << "time is " << tt << std::endl;
}

int main()
{
    Client my_client;
    std::vector<std::thread> vect;
    for(int i = 0; i < 100; i++)
    {
        vect.emplace_back(my_client);
    }
    // my_client();
    for(int i = 0; i < 100; i++)
        vect[i].join();
}
