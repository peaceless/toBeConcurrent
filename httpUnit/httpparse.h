#ifndef HTTPPARSE_H_
#define HTTPPARSE_H_
#include <string.h>
#include <string>
#include <sstream>
// #include <vector>
#include <unordered_map>
struct HttpRequest {
    bool isCGI;

    std::string url;
    std::string method;
    std::string version;

    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> GET_Args;
    std::unordered_map<std::string, std::string> POST_Args;
};
class HttpParse
{
public:
    HttpParse();
    ~HttpParse();

    void ParseData(const char *msg);
    HttpRequest &GetResult();
    bool ParseRequestLine(std::string &requestLineData);
    bool ParseURL();
    uint8_t ParseHeaders(std::string &headerData);
    bool ParseBody(std::string &bodyData);

private:

    bool CheckCGI(const std::string &url);
    bool CheckCRLF(std::string::iterator &start, std::string::iterator &end);
private:
    std::string Data;

    enum {
        requestLine,
        headers,
        body,
        error
    }status;
    struct HttpRequest httpRequest;
};

#endif