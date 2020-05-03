// parse http
#include "httpparse.h"
#include <iostream>
HttpParse::HttpParse()
{
    status = requestLine;
}
HttpParse::~HttpParse()
{}
void HttpParse::ParseData(const char *msg)
{
    // if has data has not parsed
    // combine the new data reached and the left data
    // find a \n from data has read
    std::cout << "msg is \n" << msg << "\\";
    std::cout << "\n---------\n" << std::endl;
    Data += msg;
    std::string temp;
    std::string::size_type pos = 0, breakPoint = 0;
    std::string::iterator begin,cr, lf;
    begin = Data.begin();
    while ((pos = Data.find_first_of('\n', pos)) != std::string::npos) {
        lf = begin + pos + 1;
        cr = lf - 2;
        // std::cout << *cr << *lf << int('\n')<< std::endl;
        // std::cout << int(*cr) << int(*lf) << std::endl;
        if (!CheckCRLF(cr, lf)) {
            std::cout << "get true" << std::endl;
            return ;
        }
        temp = Data.substr(breakPoint, pos + 1);
        pos ++;
        breakPoint = pos;
        std::cout << pos << "is here" << std::endl;
        switch(status) {
            case requestLine: {
                if (ParseRequestLine(temp)) {
                    status = headers;
                } else status = error;
            }
            case headers: {
                uint8_t  ret = ParseHeaders(temp);
                if (ret == 0) {
                    status = body;
                } else if (ret == -1) status = error;
            }
            case body: {
                if (ParseBody(temp)) {

                } else status = error;
            }
        }
    }
    Data.erase(0, breakPoint);
    std::cout << Data << std::endl;
    std::cout << "\n--------\n" << std::endl;
}

bool HttpParse::ParseRequestLine(std::string &data)
{
    const char *msg = data.data();
    bool flag = false;
    enum {method,
            url,
            version}status;
    status = method;
    const char *pos = nullptr;
    while ((pos = strchr(msg, ' ')) != nullptr)
    {
        if (*msg == ' ') {
            msg ++;
            continue;
        }
        switch(status) {
        case method :
            if (strncmp(msg, "GET", 3))
                return false;
            httpRequest.method.append(msg, (pos - msg));
            status = url; break;
        case url :
            httpRequest.url.append(msg, (pos - msg));
            status = version;
        case version :
            if (!strncmp(pos + 1, "HTTP", 4)) {
                int i = 0;
                const char *versionNumber = strchr(pos, '/');
                versionNumber++;
                pos = versionNumber;
                for (; *pos != '\r'; pos++, i++);
                httpRequest.version.append(versionNumber, i);
                return ParseURL();
            } else return false;
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
    if (args_start_pos == std::string::npos) {
        httpRequest.isCGI = CheckCGI(url);
    } else {
        std::string args = url.substr(args_start_pos + 1);
        url = url.substr(0, args_start_pos);
        httpRequest.isCGI = CheckCGI(url);

        std::string::size_type pos = args.find('&');
        while (pos != std::string::npos) {
            std::string argument = args.substr(0, pos);
            args = args.substr(pos + 1);

            std::string::size_type p = argument.find('=');
            if (p == std::string::npos) {
                return false;
            } else {
                httpRequest.GET_Args[argument.substr(0, p)] = argument.substr(p+1);
            }
            pos = args.find('&');
        }
        auto p = args.find('=');
        if (p == std::string::npos) {
            return false;
        } else {
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

uint8_t HttpParse::ParseHeaders(std::string &data)
{
    std::string::iterator head = data.begin();
    std::string::iterator end = data.end();
    std::string::iterator pos = head;

    std::cout << data << std::endl;
    if (CheckCRLF(head, end))
        return 0;
    std::string header_field, header_value;
    enum {field, colon, value, next, h_end, error} status;
    status = field;
    while(pos != (end - 2) && status != error) {
        switch(status) {
            case field: {
                if (!(*pos == ' ' || *pos == ':')) {
                    break;
                }
                header_field.append(head, pos);
                status = colon;
            }
            case colon: {
                if (*pos == ' ' || *pos == ':') {
                    break;
                }
                head = pos;
                status = value;
            }
            case value: {
                if (*pos != '\r') {
                    break;
                }
                header_value.append(head, pos);
                httpRequest.headers[header_field] = header_value;
                header_field.clear();
                header_value.clear();
                status = next;
            }
            case next: {
                // if (CheckCRLF(pos, end)) {
                //     pos ++;
                // } else status = error;
                break;
            }
        }
        pos ++;
    }
    if (status == error) {
        return -1;
    }
    return 1;
}
// \r\n
bool HttpParse::CheckCRLF(std::string::iterator &start, std::string::iterator &end)
{
    if (!((end - start) > 1))
        return false;
    if(*start == '\r' && *(start + 1) == '\n') {
        start ++;
        return true;
    }
    return false;
}

bool HttpParse::ParseBody(std::string &body)
{
    // std::unordered_map<std::string, std::string>::iterator
    auto content_type_data_iterator = httpRequest.headers.find("Content-Type");//["Content-Type"];
    if (content_type_data_iterator == httpRequest.headers.end())
        return true;
    std::string content_type_data = content_type_data_iterator->second;
    size_t w;
    for ( w = 0; w < content_type_data.size(); w++) {
        if (content_type_data[w] == ' ' || content_type_data[w] == ';')
            break;
    }
    std::string type = content_type_data.substr(0, w);

    if (type == "application/x-www-form-urlencoded") {
        std::string &args = body;
        auto pos = args.find('&');
        while(pos != std::string::npos) {
            auto argument = args.substr(0, pos);
            args = args.substr(pos + 1);

            auto p = argument.find('=');
            if (p == std::string::npos) {
                return false;
            } else {
                httpRequest.POST_Args[argument.substr(0,p)] = argument.substr(p + 1);
            }
            pos = args.find('&');
        }
        auto p = args.find('=');
        if (p == std::string::npos) {
            return false;
        } else {
            httpRequest.POST_Args[args.substr(0, p)] = args.substr(p + 1);
        }
    } else if (type == "multipart/form-data") {
        // auto pos = content_type_data.find("boundary");
        // if (pos != std::string::npos) {
            // /*
            //   Data Format:

            //   [Boundary]\r\n
            //   [key] : [value]\r\n
            //   \r\n
            //   \r\n
            //   [Boundary]--\r\n

            // */
        //    auto boundary = content_type_data.substr(pos);
        //    auto equal_symbol_pos = boundary.find('=');
        //    if (equal_symbol_pos == std::string::npos)
                // return false;
            // boundary = boundary.substr(equal_symbol_pos + 1);
            // size_t index = 0;
            // for (; index < boundary.size(); index ++) {
                // if (boundary[index] == ' ' || boundary[index] == ':') {
                    // break;
                // }
            // }
            // boundary = boundary.substr(0, index);

            // split body data base on boundary
            // std::vector<std::string> body_data;

            // std::stringstream ss(body);
            // std::string getData, tempData;

            // while(std::getline(ss, tempData)) {
                // the line is boundary
                // if (tempData.find(boundary) != std::string::npos)
                // {
                    // if (!getData.empty()) {
                        // body_data.emplace_back(getData);
                        // getData = "";
                    // }
                // } else {
                    // getData.append(tempData);
                    // getData.append("\n");
                // }
            // }

            // parse body data
            // for(auto &i : body_data) {
                // status = Header;
                // rawHeaders = "";
                // current_data = "";
                // std::unordered_map<std::string, std::string> headers;
                // auto c = i.begin();
                // for(; c != i.end(); c++) {
                    // Separate(*c);
                    // if (status == HeaderEnd) {
                        // ParseHeaders(headers);
                        // break;
                    // }
                // }
                // c++;
                // Get Name from Conten-Disposition
                // std::string body(c, i.end());
                // auto data = headers["Conten-Disposition"];
                // if (!data.empty()) {
                    // auto name_pos = data.find("name");
                    // if (name_pos == std::string::npos)
                        // continue;
                    // auto value_pos = data.find('\"',name_pos) + 1;
                    // if (value_pos == std::string::npos)
                        // continue;
                    // std::string value;
                    // size_t n = value_pos;
                    // for (; n != std::string::npos; n++) {
                        // if (data[n] == '\"')
                            // break;
                    // }
                    // set the value
                    // value = data.substr(value_pos, n - value_pos);
                    // httpRequest.POST_Args[value] = body;
                    // std::cout << "post value size:" << httpRequest.POST_Args[value].size() << std:endl;
                    // std::cout << "body size:" << body.size();
                    // std::fstream file("tt.ico", std::ios::binary | std::ios::out);
                    // file.write(body.c_str(), body.size());
                    // file.close();
                    // std::cout << "save the file";
                // }
            // }
        // } else return false;
    } else if (type == "text/plain") {
        httpRequest.POST_Args["data"] = body;
        return true;
    }
    return true;
}

HttpRequest &HttpParse::GetResult()
{
    return httpRequest;
}