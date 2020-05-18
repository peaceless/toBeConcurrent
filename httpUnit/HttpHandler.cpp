#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include "HttpHandler.h"
#include "fast_cgi/fcgi.h"

#include <iostream>

extern std::string SERVER_WEB_PATH;

HttpHandler::HttpHandler(int sock)
{
    socket = sock;
}
bool HttpHandler::Handle (const char *buffer/*, Cache* &mcache*/)
{
    uint8_t status = 0;
    // cache = mcache;
    status = ParseHttpRequest(buffer);
    if (status == 1) {
        if (request.url == "/") {
            request.url = "/index.html";
            status = GetFile();
        // } else if (request.isCGI) {
        //     status = HandleCGIRequest("./www");
        } else {
            status = GetFile();
        }
        status = SendResponse(socket, status);
    }
    return status;
}

bool HttpHandler::GetFile()
{
    response.data = "";
    struct stat buffer;
    std::string path("./www" + request.url);
    if(request.method == "POST") {
        if (request.POST_Args["id"] == "test" && request.POST_Args["pwd"] == "123") {
            path = "./www/rlogin.html";
        } else path = "./www/wlogin.html";
    }
    if (stat(path.c_str(), &buffer)) {
        std::cout << path << ": file not exist" << std::endl;
        return false;
    }
    std::cout << path << std::endl;
    std::ifstream file(path, std::ios_base::binary);
    std::string data;
    while(getline(file, data)) {
        response.data += data;
        std::cout << data;
    }
    file.close();
    return true;
    // return cache->GetFile(request.URL, response.data);
}

int8_t HttpHandler::ParseHttpRequest (const char *buffer)
{
    // HttpParse parser;
    int8_t ret = 0;
    ret = parser.ParseData(buffer);
    if (ret == 1) {
        request = parser.GetResult();
    }
    return ret;
}

bool HttpHandler::HandleCGIRequest (const std::string& root_path)
{
    FastCgi fc;
    fc.setRequestId(1);
    fc.startConnect();
    fc.sendStartRequestRecord();
    fc.sendParams("SCRIPT_FILENAME", root_path + request.url);
    fc.sendParams("REQUEST_METHOD", request.method);
    for (auto i : request.headers) {
        fc.sendParams(i.first, i.second);
    }
    for (auto i : request.GET_Args) {
        // LOG_TRACE(i.first, ":", i.second);
        fc.sendParams(i.first, i.second);
    }
    for (auto i : request.POST_Args) {
        // LOG_TRACE(i.first, ":", i.second);
        fc.sendParams(i.first, i.second);
    }
    fc.sendEndRequestRecord();
    fc.readFromPhp();
    response.data = fc.getHTMLData();
    return true;
}

bool HttpHandler::SendResponse (int socket, bool status)
{
    if (status) {
        response.status_code = "200 OK";
    } else {
        response.status_code = "404 NOT FOUND";
    }
    std::string m_response =
        "HTTP/1.1 " + response.status_code + "\r\n"
        + "Content-Type: text/html;charset=UTF-8\r\n"
        + "Content-Length: " + std::to_string(response.data.size()) + "\r\n\r\n"
        + response.data;
    std::cout << "write in.\n" << std::endl;
    int ret = write(socket, m_response.c_str(), m_response.size());
    std::cout << "over.\n" << std::endl;
    if (ret == -1) {
        std::cout << ret << "ret is." << std::endl;
        shutdown(socket, SHUT_WR);
        return false;
    }
    return true;
}
