#ifndef CACHE_H_
#define CACHE_H_

#include <string>

class Cache
{
public:
    Cache() = default;

    virtual bool GetFile(const std::string& file_path,
                         std::string& file_data) = 0;

    virtual std::string GetRootPath() = 0;

    virtual void UpdateCache() = 0;
};

#endif
