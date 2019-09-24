#ifndef OBJECTASTHREADARG_H
#define OBJECTASTHREADARG_H
#include <thread>
#include <iostream>
class ObjectAsThreadArg {
public :
    void hello(int x)
    {
        std::cout << "hello" << std::endl;
    }
    ObjectAsThreadArg(const ObjectAsThreadArg &)
    {
        std::cout << "run copy func" << std::this_thread::get_id() << std::endl;
    }
    ObjectAsThreadArg & operator=(const ObjectAsThreadArg & test)
    {
        std::cout << "run = func" << std::endl;
        if (this != &test)
            return *this;
        return *this;
    }
//    void operator()()
//    {
//        std::cout << "run ()." << std::endl;
//    }
    ObjectAsThreadArg()
    {
        std::cout << "construct run" << std::endl;
    }
    ~ObjectAsThreadArg()
    {
        std::cout << "destruct run" << std::endl;
    }
};
#endif // OBJECTASTHREADARG_H
