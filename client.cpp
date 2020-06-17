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

#include "include/openssl/ssl.h"
#include "include/openssl/err.h"

class Client
{
public:
    Client(SSL_CTX *ctx, std::string server_ip = "127.0.0.1");
    void operator()();
    //void connecting();

private:
    sockaddr_in serv_addr;
    SSL_CTX *m_ctx;
};

Client::Client(SSL_CTX *ctx, std::string server_ip)
{
    m_ctx = ctx;
    const char *ip = server_ip.c_str();
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(8080);
}

void Client::operator()()
{
    SSL *ssl;
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
            ssl = SSL_new(m_ctx);
            SSL_set_fd(ssl, sock);
            if (SSL_connect(ssl) == -1) {
                std::cout << "erro in ssl_connect" << std::endl;
                return ;
            }
            char remsg[100] = "\0";
            // std::cout << sock << "++" << std::endl;
            std::clock_t start = clock();
            SSL_write(ssl, "GET /index.html HTTP/1.1\r\n\r\n", 28);
            SSL_read(ssl, remsg, 100);
            std::clock_t resumeTime = clock() - start;
            total_time += resumeTime;
            // while(recv(sock, remsg, 100, 0) > 0);
            sleep(1);
            SSL_shutdown(ssl);
            SSL_free(ssl);
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
    SSL_CTX *ctx;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        std::cout << "error in create ctx" << std::endl;
        exit(1);
    }
    Client my_client(ctx);
    std::vector<std::thread> vect;
    for(int i = 0; i < 100; i++)
    {
        vect.emplace_back(my_client);
    }
    // my_client();
    for(int i = 0; i < 100; i++)
        vect[i].join();
    SSL_CTX_free(ctx);
}
