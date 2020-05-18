#include "LRUCache.hpp"
#include <utility>
#include <sstream>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

bool LRUCache::GetFile(const std::string& key, std::string& result)
{
    auto iter = cache_map.find(key);
    if(iter == cache_map.end()) {
        if (!GetFileFromDisk(key)) {
            return false;
        }
        iter = cache_map.find(key);
    }
    result = iter->second->second;
    cache_list.splice(cache_list.begin(), cache_list, iter->second);
    return true;
}

bool LRUCache::Put(const std::string& key, const std::string& value)
{
    int data_size = value.size();
    if (data_size > max_size) {
        return false;
    }

    if (use_size + data_size <= max_size) {
        cache_list.emplace_front(std::make_pair(key, value));
        cache_map[key] = cache_list.begin();
        use_size += data_size;
    } else {
        while (use_size > max_size - data_size) {
            auto iter = cache_list.back();
            auto key = iter.first;

            use_size -= iter.second.size();
            cache_map.erase(key);
            cache_list.pop_back();
        }

        cache_list.emplace_front(std::make_pair(key, value));
        cache_map[key] = cache_list.begin();
        use_size += data_size;
    }
    return true;
}

bool LRUCache::GetFileFromDisk(const std::string &filename)
{
    struct stat buffer;
    std::string path(root_path + filename);
    if (stat(path.c_str(), &buffer)) {
        std::cout << path << ": file not exist" << std::endl;
        return false;
    }
    if (cache_map.find(filename) == cache_map.end()) {
        std::lock_guard<std::mutex> lk(mut);
        if (cache_map.find(filename) == cache_map.end()) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            Put(filename, std::move(ss.str()));
        }
    }
    return true;
}

void LRUCache::UpdateCache()
{
    for (auto i = cache_list.begin(); i != cache_list.end(); i++) {
        std::ifstream file(root_path + i->first);
        std::cout << "update " << i->first << std::endl;
        std::stringstream ss;
        ss << file.rdbuf();
        i->second = ss.str();
    }
}

