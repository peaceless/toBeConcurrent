#ifndef LRUCACHE_HPP_
#define LRUCACHE_HPP_

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>

#include "cache.hpp"

struct DataSize {
    explicit DataSize(unsigned int msize)
        : size(msize) {}
    unsigned int size;
};

struct Byte : DataSize {
    explicit Byte(int msize)
        : DataSize(msize) {}
};

struct MB : Byte {
    explicit MB(unsigned int msize)
        : Byte(msize * 1024) {}
};

struct GB : MB {
    explicit GB(unsigned int msize)
        : MB(msize * 1024) {}
};

class LRUCache : public Cache
{
    using KV = std::pair<std::string, std::string>;
    using KV_list_iter = std::list<KV>::iterator;

public:
    explicit LRUCache(const std::string &path, DataSize data_size)
        : Cache(), root_path(path), max_size(data_size.size), use_size(0) {};

    bool GetFile(const std::string& key, std::string& result);

    bool Put(const std::string& key, const std::string& value);

    std::string GetRootPath()
    {
        return root_path;
    }

    void UpdateCache();

private:
    bool GetFileFromDisk(const std::string& filename);

    std::mutex mut;
    std::string root_path;
    unsigned int max_size;
    unsigned int use_size;
    std::list<KV> cache_list;
    std::unordered_map<std::string, KV_list_iter> cache_map;

};


#endif
