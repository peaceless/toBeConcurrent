// #include "loopEvent.h"
// loopEvent::loopEvent(int fd, std::shared_ptr<ThreadSafeQueue<int>> th, ThreadPool *tp)
//     : epfd(fd), fd_queue(th), Event(fd), pool(tp), timeCount(this)
// {
//     events_queue.resize(LIMIT); // all place has a null value
//     events_queue[0] = new pEvent(0, this);
// }
// void loopEvent::addErrorEvent(int fd)
// {
//     std::lock_guard<std::mutex> lk(mut);
//     error_queue.push(fd);
// }

// #define LIMIT_PER_TIME 10
// void loopEvent::handle()
// {
//     int max = 0;
//     epfd = epoll_create(500);
//     while (1)
//     {
//         for (int i = 0; i < LIMIT_PER_TIME; i++)
//         {
//             std::shared_ptr<int> event_fd = fd_queue->TryPop();
//             if (!event_fd) // if queue is empty
//                 break;

//             event.data.fd = *event_fd;
//             event.events = EPOLLIN | EPOLLET;
//             if (epoll_ctl(epfd, EPOLL_CTL_ADD, *event_fd, &event) == -1)
//                 lg << __func__ << " error: add event into epoll wrong!" << *event_fd << std::endl;

//             if (events_queue[*event_fd] != nullptr)
//             {
//                 lg << "-1 error." << std::endl;
//                 lg << *event_fd << "is exit." << std::endl;
//                 exit(-1);
//             }
//             events_queue[*event_fd] = new pEvent(*event_fd, this);
//             events_queue[*event_fd]->setNonBlock();
//             events_queue[0]->sockfd++;

//             if(max < *event_fd) max = *event_fd;
//         }

//         int eventCount = epoll_wait(epfd, events, LIMIT, LIMIT);

//         timeCount.Timer(events, eventCount);
//         for (int i = 0; i < eventCount; ++i)
//         {
//             // XXX : epollerr & epollhup seem like only hanppen when server peer
//             if (events[i].events == (EPOLLERR | EPOLLHUP))
//             {
//                 exit(-1);
//             } else if (events[i].events == EPOLLIN)
//             {
//                 std::lock_guard<std::mutex> lk(events_queue[events[i].data.fd]->mut);
//                 if (events_queue[events[i].data.fd]->evcount > 0)
//                 {
//                     events_queue[events[i].data.fd]->evcount ++;
//                 } else if (events_queue[events[i].data.fd]->evcount == 0) {
//                     events_queue[events[i].data.fd]->evcount ++;
//                     pool->enqueue(&pEvent::handle, events_queue[events[i].data.fd]);
//                 }
//             }
//         }
//         cleanErrorEvent();
//     }
// }

// void loopEvent::cleanErrorEvent()
// {
//     std::lock_guard<std::mutex> lk(mut);
//     int t = error_queue.size();
//     for (int i = 0; i < LIMIT && i < t; i++)
//     {
//         int p = error_queue.front();
//         error_queue.pop();
//         removeEvent(p);
//     }
// }

// void loopEvent::removeEvent(int event_id)
// {
//     if (events_queue[event_id] == nullptr)
//         return;
//     // wait for all thread has handle over this sock
//     while (events_queue[event_id]->evcount > 0)
//         std::this_thread::yield;
//     event.data.fd = events_queue[event_id]->sockfd;
//     delete events_queue[event_id];
//     events_queue[event_id] = nullptr;

//     if (epoll_ctl(epfd, EPOLL_CTL_DEL, event_id, &event))
//     {
//         lg << "...................delete error" << errno << std::endl;
//         exit(-1);
//     }
//     shutdown(event_id, SHUT_RDWR);
//     close(event_id);
//     events_queue[0]->sockfd--;
// }