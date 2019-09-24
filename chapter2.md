## 章节2 线程管理
### 启动线程

​	构造thread对象，该对象创建时就启动一个新的线程

​	thread构造函数以一个函数为参数，即新线程执行该函数的内容，在函数执行结束时，线程也结束

​	thread可调用类型构造，将带有函数调用符类型的类的实例传入thread类中，替换thread的默认构造函数，例如：

```cpp
class bakground_task
{
public :
    void operator()() const
    {
        do_something();
        do_something_els();
    }
};
background_task f;
std::thread my_thread(f);
```
 	thread构造函数中提供的函数对象会复制到新线程的存储空间中，函数对对象的执行和调用都会在线程的内存空间中进行。

​	thread构造函数中如果传递临时变量，编译器会将其解析为函数声明，而非thread对象的定义，例如:

```cpp
std::thread my_thread(background_task());
```

此处相当于声明了一个名为my_thread的函数，其中background_task()被认为是一个没有参数且返回值为background_task对象的函数，my_thread函数返回一个thread对象。

​	如果要使用临时对象作为thread的参数可以使用多组括号或新统一的初始化语法以及lambda表达式解决这个

