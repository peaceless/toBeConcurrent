// #include "mytimer.h"
// #include "loopEvent.h"

// TimeWheel::TimeWheel(loopEvent *le)
//     : le(le), tik(0)
// {
//     insertFlag = false;
//     conns.resize(500, 0);
//     wheel.resize(8);
// }
// TimeWheel::~TimeWheel()
// {
// }
// void TimeWheel::Timer(epoll_event epollEvents[], int eventCount)
// {
//     if (insertFlag) {
//         tik ++;
//         tik = tik % 8;
//     }
//     int sock;
//     for (int i = 0; i < eventCount; i++) {
//         sock = epollEvents[i].data.fd;
//         int tail = (tik + 7)%8;
//         if (conns[sock] != 0) {
//             int t = conns[sock];
//             wheel[t].erase(sock);
//         }
//         std::cout << "insert " << sock << " into " << tail << "." << std::endl;
//         wheel[tail].insert(sock);
//         events[sock] = tail;
//     }
//     for (int i : wheel[0]) {
//         le->addErrorEvent(i);
//     }
//     if (insertFlag) {
//         wheel[tik].clear();
//     }
//     insertFlag = !insertFlag;
// }