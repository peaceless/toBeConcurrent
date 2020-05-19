// parse http
#include "../event/logger.hpp"
#include "httpparse.h"
#include <iostream>
#include <vector>
#include <fstream>
HttpParse::HttpParse()
{
    status = requestLine;
}
HttpParse::~HttpParse()
{
}
int8_t HttpParse::ParseData(const char *msg)
{
    // if has data has not parsed
    // combine the new data reached and the left data
    // find a \n from data has read
    // std::cout << "msg is \n" << msg << "\\";
    // std::cout << "\n---------\n" << std::endl;
    if (status == end)
    {
        status = requestLine;
        httpRequest.headers.clear();
        httpRequest.GET_Args.clear();
        httpRequest.POST_Args.clear();
        httpRequest.method = "";
        httpRequest.url = "";
        httpRequest.version = "";
        httpRequest.isCGI = false;
    }
    Data += msg;
    std::string temp;
    std::string::size_type pos = 0, breakPoint = 0;
    std::string::iterator begin, cr, lf;
    begin = Data.begin();
    if (status != body)
        while ((pos = Data.find_first_of('\n', pos)) != std::string::npos)
        {
            lf = begin + pos + 1;
            cr = lf - 2;
            if (!CheckCRLF(cr, lf))
            {
                LOG_ERROR("unexcepted character.");
                return false;
            }
            temp = Data.substr(breakPoint, pos - breakPoint - 1);
            pos++;
            breakPoint = pos;
            switch (status)
            {
            case requestLine:
            {
                if (ParseRequestLine(temp))
                {
                    status = headers;
                }
                else{
                    status = error;
                    LOG_ERROR("RequestLine error");
                }
                break;
            }
            case headers:
            {
                uint8_t ret = ParseHeaders(temp);
                if (ret == 0)
                {
                    status = body;
                    goto body_place;
                }
                else if (ret == -1) {
                    LOG_ERROR("RequestLine error");
                    status = error;
                }
                break;
            }
                // case body: {
                //     std::cout << "body parsehello." << std::endl;
                //     if (ParseBody(temp)) {

                //     } else status = error;
                //     break;
                // }
            }
        }
body_place:    Data.erase(0, breakPoint);
    if (status == body)
    {
        int8_t result = ParseBody(Data);
        if (result == 1)
        {
            status = end;
            return true;
        }
        else if(result == 0)
            status = error;
    }
    if (status == error) {
        LOG_ERROR("parse data wrong.");
        return false;
    }
    return -1;
}

bool HttpParse::ParseRequestLine(std::string &data)
{
    const char *msg = data.data();
    bool flag = false;
    enum
    {
        method,
        url,
        version
    } status;
    status = method;
    const char *pos = nullptr;
    while ((pos = strchr(msg, ' ')) != nullptr)
    {
        if (*msg == ' ')
        {
            msg++;
            continue;
        }
        switch (status)
        {
        case method:
            if (!(strncmp(msg, "GET", 3) || strncmp(msg, "POST", 4))) {
                return false;
            }
            httpRequest.method.append(msg, (pos - msg));
            status = url;
            break;
        case url:
            httpRequest.url.append(msg, (pos - msg));
            status = version;
        case version:
            if (!strncmp(pos + 1, "HTTP", 4))
            {
                int i = 0;
                const char *versionNumber = strchr(pos, '/');
                versionNumber++;
                pos = versionNumber;
                for (; *pos != '\0'; pos++, i++)
                    ;
                httpRequest.version.append(versionNumber, i);
                return ParseURL();
            }
            else
                return false;
        }
        pos += 1;
        msg = pos;
    }
    return false;
}

bool HttpParse::ParseURL()
{
    std::string &url = httpRequest.url;
    std::string::size_type args_start_pos = url.find_first_of('?');
    if (args_start_pos == std::string::npos)
    {
        httpRequest.isCGI = CheckCGI(url);
    }
    else
    {
        std::string args = url.substr(args_start_pos + 1);
        url = url.substr(0, args_start_pos);
        httpRequest.isCGI = CheckCGI(url);

        std::string::size_type pos = args.find('&');
        while (pos != std::string::npos)
        {
            std::string argument = args.substr(0, pos);
            args = args.substr(pos + 1);

            std::string::size_type p = argument.find('=');
            if (p == std::string::npos)
            {
                return false;
            }
            else
            {
                httpRequest.GET_Args[argument.substr(0, p)] = argument.substr(p + 1);
            }
            pos = args.find('&');
        }
        auto p = args.find('=');
        if (p == std::string::npos)
        {
            return false;
        }
        else
        {
            httpRequest.GET_Args[args.substr(0, p)] = args.substr(p + 1);
        }
    }
    return true;
}

bool HttpParse::CheckCGI(const std::string &url)
{
    auto type_start_pos = url.find_last_of('.');
    auto type = url.substr(type_start_pos + 1, std::string::npos);
    return type == "php";
}

int8_t HttpParse::ParseHeaders(std::string &data)
{
    std::string::iterator head = data.begin();
    std::string::iterator end = data.end();
    std::string::iterator pos = head;

    if (data.size() == 0)
    {
        LOG_INFO("parse header is end.");
        return 0;
    }
    std::string header_field, header_value;
    enum
    {
        field,
        colon,
        value,
        next,
        h_end,
        error
    } status;
    status = field;
    while (pos != end && status != error)
    {
        switch (status)
        {
        case field:
        {
            if (!(*pos == ' ' || *pos == ':'))
            {
                break;
            }
            header_field.append(head, pos);
            status = colon;
            break;
        }
        case colon:
        {
            if (*pos == ' ' || *pos == ':')
            {
                break;
            }
            head = pos;
            pos--;
            status = value;
            break;
        }
        // TODO: without \r\n,put value into map
        case value:
        {
            if (end - pos > 1)
            {
                break;
            }
            header_value.append(head, pos + 1);
            httpRequest.headers[header_field] = header_value;
            header_field.clear();
            header_value.clear();
            // status = next;
            break;
        }
            // case next: {
            //     // if (CheckCRLF(pos, end)) {
            //     //     pos ++;
            //     // } else status = error;
            //     break;
            // }
        }
        pos++;
    }
    if (status == error)
    {
        return -1;
    }
    return 1;
}
// \r\n
bool HttpParse::CheckCRLF(std::string::iterator &start, std::string::iterator &end)
{
    if (!((end - start) > 1))
        return false;
    if (*start == '\r' && *(start + 1) == '\n')
    {
        start++;
        return true;
    }
    return false;
}

void ParseFormData(std::string &body)
{

}
// zero for false, one for over, -1 for data not enough
int8_t HttpParse::ParseBody(std::string &body)
{
    // std::unordered_map<std::string, std::string>::iterator
    bool isGET = false;
    if (httpRequest.method == "GET")
        isGET = true;
    auto content_type_data_iterator = httpRequest.headers.find("Content-Type");
    if (content_type_data_iterator == httpRequest.headers.end()) {
        if (!isGET)
            return 0;
    }
    auto content_data_len_iterator = httpRequest.headers.find("Content-Length");
    if (content_data_len_iterator == httpRequest.headers.end())
        return 0;
    auto len = stoi(content_data_len_iterator->second);
    if (body.size() < len)
        return -1;
    auto data = body.substr(0, len);
    body = body.substr(len);
    std::string content_type_data = content_type_data_iterator->second;
    size_t w;
    for (w = 0; w < content_type_data.size(); w++)
    {
        if (content_type_data[w] == ' ' || content_type_data[w] == ';')
            break;
    }
    std::string type = content_type_data.substr(0, w);

    if (type == "application/x-www-form-urlencoded")
    {
        std::string &args = data;
        auto pos = args.find('&');
        while (pos != std::string::npos)
        {
            auto argument = args.substr(0, pos);
            args = args.substr(pos + 1);

            auto p = argument.find('=');
            if (p == std::string::npos)
            {
                return false;
            }
            else
            {
                httpRequest.POST_Args[argument.substr(0, p)] = argument.substr(p + 1);
            }
            pos = args.find('&');
        }
        auto p = args.find('=');
        if (p == std::string::npos)
        {
            return false;
        }
        else
        {
            httpRequest.POST_Args[args.substr(0, p)] = args.substr(p + 1);
        }
    }
    else if (type == "multipart/form-data")
    {
        auto pos = content_type_data.find("boundary");
        if (pos != std::string::npos) {
            /*
              Data Format:

              [Boundary]\r\n
              [name]="name"\r\n
              \r\n
              value
              [Boundary]--\r\n

            */
            //Get Boundary
            auto boundary = content_type_data.substr(pos);
            auto equal_symbol_pos = boundary.find('=');
            if (equal_symbol_pos == std::string::npos) {
                return false;
            }
            boundary = boundary.substr(equal_symbol_pos+1);
            size_t index = 0;
            for (; index < boundary.size(); index++) {
                if (boundary[index] == ' ' || boundary[index] ==';') {
                    break;
                }
            }
            boundary = boundary.substr(0, index);
            boundary = "--" + boundary;
            std::string::size_type len = boundary.size();

            //Split Body Data Base On Boundary
            std::string endData(boundary + "--\r\n");
            enum {m_boundary, m_field, m_space,m_value, m_end, m_error} status;
            status = m_boundary;
            std::string::size_type nameStartPos;
            std::string field;
            while(status != m_error) {
                pos = data.find("\r\n");
                if (pos != std::string::npos) {
                    std::string temp = data.substr(0, pos);
                    data = data.substr(pos + 2);
                    switch(status) {
                        case m_boundary: {
                            if(temp == boundary) {
boundary:                                status = m_field;
                                break;
                            } else return 0;
                        }
                        case m_field: {
                            if (temp.size() == 0) {
                                status = m_value;
                                break;
                            }                    
                            if ((pos = temp.find("name")) != std::string::npos) {
                                nameStartPos = temp.find('\"', pos);
                                if (nameStartPos != std::string::npos) {
                                    nameStartPos ++;
                                    pos = temp.find('\"', nameStartPos);
                                    if (pos == std::string::npos) {
                                        LOG_ERROR("no namein");
                                        return 0;
                                    }
                                    field = temp.substr(nameStartPos, pos - nameStartPos);
                                }
                            }
                            break;
                        }
                        case m_value: {
                            httpRequest.POST_Args[field] += temp;
                            status = m_end;
                            break;
                        }
                        case m_end: {
                            if (temp == boundary + "--") {
                                return 1;
                            }
                            if (temp == boundary) {
                                goto boundary;
                            }
                            break;
                        }
                    }
                } else status = m_error;
            }
            return -1;
            // pos = data.find(boundary + "--\r\n");
            // if (pos != std::string::npos) {
            //     httpRequest.POST_Args[field] += data.substr(0, pos);
            //     std::cout << httpRequest.POST_Args[field] << std::endl;
            // } else return -1;
        }
    }
    else if (type == "text/plain")
    {
        httpRequest.POST_Args["data"] = data;
    }
    return 1;
}

HttpRequest &HttpParse::GetResult()
{
    return httpRequest;
}