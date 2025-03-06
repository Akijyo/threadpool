# 线程池项目

这是一个用C++实现的线程池项目。线程池可以管理多个线程，执行并发任务，提高程序的执行效率。

## 主要功能

- 动态增加和销毁线程
- 提交任务到线程池
- 等待所有任务完成

## 使用说明

- `static ThreadPool* getThreadPool(int minThreadNum, int maxThreadNum)`: 获取线程池对象
-- minThreadNum指定线程池最小线程数，maxThreadNum指定线程池最大线程树，通过调用该函数获取单例对象

- `bool addTask(std::function<void()> task)`: 添加任务到任务队列
-- 调用此函数将任务函数加入线程池，如果任务函数有参数需要使用std::bind()去将任务函数和参数绑定

- `template <typename Func, typename... Args> auto submit(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>`: 提交任务并获取返回值
-- 异步线程池实现的函数，也是一个提交任务的函数，函数执行返回一个std::futrue类。对比上一个addTask函数相比，此函数不需要std::bind()去绑定参数，而且可以获取返回值

- `void waitAllTasksDone()`: 等待所有任务执行完毕
-- 等待线程池任务执行完毕，在主程序中调用防止主线程提前退出
