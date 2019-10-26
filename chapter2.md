## 章节2 线程管理
### 启动线程
    - 构造thread对象，该对象创建时即启动一个新的线程thread构造函数以一个函数为参数，即新线程执行该函数的内容，在函数结束时，线程随之结束
    - thread可调用类型构造，将带有函数调用符类型的类的实例传入thread类中，替换thread的默认构造，例如：
    `cpp`
    ``
## 章节3 线程间共享数据
### 共享数据带来的问题
    对于共享数据执行操作的结果取决于线程执行的顺序
### 使用互斥量保护共享数据
    以一个标志位标识是否有对共享数据正在操作，这个标识位就是锁(std::mutex),在需要操作的数据前上锁lock(),在操作数据后解锁unlock()，在操作过程中如果意外退出该操作，也应该进行unlock()，否则其它线程无法获取锁.这个时候使用std::lock_guard对象保有锁通常是更好的选择：lock_guard对象在建立时上锁，在对象释放时也释放了锁，而c++程序保证了，建立的对象总会被释放，故lock_guard对象比普通的mutex对象更方便和安全。
### 使用代码保护共享数据
    锁没办法保证函数不返回操作对象的引用或者指针，如果一个函数返回了共享数据的引用或指针，那么就可以在这个函数外使用共享数据，从而避开锁的保护，所以，应该在编写代码时避免此类操作。  
    - 发现接口的内在竞争
    在多个线程同时对共享数据操作时，他们都对共享数据进行了一组操作，那么程序的运行结果有可能会依赖于各个线程的执行顺序，即对于每步操作的上锁在一组操作内没有起到锁定操作的结果。
    lock_guard参数可以选择adopt_lock，从而直接获取已有的上锁对象mutex；
    - 死锁
    若两个线程互相等待对方的锁就会造成两者都无法执行，即死锁。
        互斥量总是以相同的顺序上锁或者要么同时获得所有锁，要么一个锁都不要。使用std::lock()方法可以同时获取一组锁。
        避免嵌套锁：在持有一个锁的情况下，不去争取第二个锁；不要在持有锁时调用用户提供的操作；以固定的顺序获取锁；使用层次锁。
### std::unique_lock
    unique_lock比lock_guard更为灵活，unique_lock可以在任何时候调用unlock释放已持有的锁，且unique_lock对象支持转移(std::move)而不支持复制，可以将锁的所有权转移给一个unique_lock对象。
    其中，unique_lock的lock函数更新标志以确定该对象是否拥有特定的互斥量，该标志是为了确保unlock()在unique_lock对象的析构函数被正确的调用，也因此unique_lock对象比lock_guard占用的内存更大。
## 保护共享数据的替代设施
### 保护共享数据的初始化过程
    声名狼藉的双重检查锁模式：
    ```cpp
    void undefined_behaviour_with_double_checked_locking()
    {
        if(!resource_ptr) // 1
        {
            std::lock_guard<std::mutex> lk(resource_mutex);
            if (!resource_ptr) // 2
            {
                resource_ptr.reset(new some_resource);
            }
        }
        resource_ptr->do_something();// 3
    }
    ```
    未被锁保护的操作1没有与其它线程理被锁保护的写入操作3进行同步。因此会产生条件竞争，这个条件竞争不但覆盖指针本身，还会影响到其所指的对象；即使一个线程知道另一个线程完成对指针的写入，也可能没有看到新创建的some_resource实例，导致调用do_something()的到不正确结果，于第5章内存模型有解释....
    使用std::call_once获得线程安全的初始化操作。
        std::once_flag resouce_flag;
        std::call_once(resource_flag,func);
### 保护很少更新的数据结构
    读写锁：
    ```cpp
    read()
    {
        boost::shared_lock<boost::shared_mutex> lk(mutex);
    }
    write()
    {
        std::lock_guard<boost::shared_mutex> lk(mutex);
    }
    ```
### 嵌套锁
    一个线程对一个std::mutex对象多次上锁，即std::recursive_mutex类，lock多少次就要unlock多少次.