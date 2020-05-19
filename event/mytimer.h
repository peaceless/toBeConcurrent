#ifndef MYTIMER_H
#define MYTIMER_H
#include <vector>
#include <set>
#include <sys/epoll.h>
#include "event.h"
// #include "loopEvent.h"
class loopEvent;
class TimeWheel{
public:
    explicit TimeWheel(loopEvent *le);
    ~TimeWheel();

    void Timer(epoll_event epollEvents[], int eventCount);
    void Remove(int sock);
private:
    loopEvent* le;
    int8_t tik;
    bool insertFlag;
    std::vector<int> events;
    std::vector<int> conns;
    std::vector<std::set<int>> wheel;
};
#endif // MYTIMER_H