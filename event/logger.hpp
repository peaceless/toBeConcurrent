#ifndef LOGGER_H_
#define LOGGER_H_

#include <condition_variable>
#include <queue>
#include <sstream>
#include <iostream>
#include <thread>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#define INFO __LINE__,__FUNCTION__
#define __LOG Logger::GetInstance()->Log

#define LOG_INFO(...) __LOG(INFO, 0, __VA_ARGS__)
#define LOG_TRACE(...) __LOG(INFO, 1, __VA_ARGS__)
#define LOG_WARM(...) __LOG(INFO, 2, __VA_ARGS__)
#define LOG_ERROR(...) __LOG(INFO, 3, __VA_ARGS__)

#define LOG_SETLIMIE(i) Logger::getInstance()->setlimit(i)
#define LOG_DORECORD() Logger::getInstance()->doRecord()
#define LOG_EXIT() Logger::getInstance()->exit()

class Logger
{
public:
    static Logger* GetInstance ()
    {
        static Logger logger;
        return &logger;
    }

    template<typename... T>
    void Log(int line, const char *function, int lv, const T... msg)
    {
        std::stringstream ss;
        Log(ss, lv,'{',lvs[lv].text,"}",'<',function,'-',line,">:",msg...);
    }

    template<typename S, typename... T>
    void Log(std::stringstream& ss, int lv, const S s, const T... p)
    {
        ss << s;
        Log(ss, lv, p...);
    }

    template<typename T>
    void Log(std::stringstream& ss, int lv,const T p)
    {
        std::unique_lock<std::mutex> lk(mutex);
        ss << p;
        std::cout << "\033[1;"
                  << lvs[lv].fg << ';' << lvs[lv].bg << 'm'
                  << ss.str()
                  << "\033[0m"
                  << std::endl;
        if (!do_record) {
            ss.clear();
            return;
        }
        log_queue.push(ss.str());
        ss.clear();
        count++;
        if (count == limit) {
            ready = true;
            count = 0;
            while(ready) {
                lk.unlock();
                condition.notify_one();
                std::this_thread::yield();
                lk.lock();
            }
        }
    }

    void Exit ()
    {
        if (!do_record) {
            return;
        }
        std::unique_lock<std::mutex> lk(mutex);
        stop = true;
        while(stop) {
            lk.unlock();
            condition.notify_one();
            std::this_thread::yield();
            lk.lock();
        }
        while (!log_queue.empty()) {
            record_file << log_queue.front() << '\n';
            log_queue.pop();
        }
        record_file.close();
    }

    void Write ()
    {
        std::unique_lock<std::mutex> lk(mutex, std::defer_lock);
        while (!stop) {
            lk.lock();
            condition.wait(lk, [this] {return ready || stop;});
            ready = false;
            std::queue<std::string>::reference pos = log_queue.back();
            lk.unlock();
            while(log_queue.front() != pos) {
                record_file << log_queue.front() << '\n';
                log_queue.pop();
            }
        }
        lk.lock();
        stop = false;
        lk.unlock();
    }

    void Setlimit (int i)
    {
        limit = i;
    }

    bool DoRecord ()
    {
        do_record = true;
        if (!record_file.is_open()) {
            record_file.clear();
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "%Y_%m_%d_%H_%M_%S.log");
            record_file.open(ss.str(), std::ios::out);
            if(record_file.is_open()) {
                std::thread t([this] { Write();});
                t.detach();
                return true;
            }
            do_record = false;
            return false;
        }
        return true;
    }

private:
    Logger()
        : limit(100), ready(false), count(0),
          stop(false), do_record(false) {}

    struct Level {
        int fg;
        int bg;
        std::string text;
    };

    Level lvs[5] = {
        {37,40,"info"},
        {36,40,"trace"},
        {33,40,"warm"},
        {37,41,"error"}
    };

    std::mutex mutex;
    std::condition_variable condition;

    int limit;
    bool ready;
    int count;
    bool stop;
    bool do_record;

    std::queue<std::string> log_queue;
    std::fstream record_file;
};

#endif
