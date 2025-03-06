#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <atomic>
#include <map>
#include <vector>
#include <chrono>
#include <future>
class ThreadPool
{
private:
    std::thread addThreadID;                         // 添加者线程的ID
    std::thread destroyThreadID;                     // 销毁者线程的ID
    std::map<std::thread::id, std::thread> workerID; // 用map存放工作线程的ID
    std::vector<std::thread::id> exitID;             // 销毁线程的ID
    std::deque<std::function<void(void)>> taskQueue; // 任务队列
    std::atomic<int> minThreadNum;                   // 最小线程数
    std::atomic<int> maxThreadNum;                   // 最大线程数
    std::atomic<int> busyThreadNum;                  // 忙线程数
    std::atomic<int> liveThreadNum;                  // 存活线程数
    std::mutex mtx;                                  // 线程池的互斥锁
    std::mutex exitMtx;                              // 销毁线程的互斥锁
    std::condition_variable consumerCond;            // 消费者条件变量
    std::atomic<bool> isRun;                         // 线程池是否运行
    std::atomic<bool> exitThread;                    // 是否退出线程
    ThreadPool(int minThreadNum, int maxThreadNum);  // 构造函数
    ~ThreadPool();                                   // 析构函数
    void workThread();                               // 工作线程
    void addThread();                                // 增加工作线程的线程
    void destroyThread();                            // 删除工作线程的线程
public:
    ThreadPool(const ThreadPool &) = delete;                              // 禁用拷贝构造函数
    ThreadPool &operator=(const ThreadPool &) = delete;                   // 禁用赋值运算符
    static ThreadPool *getThreadPool(int minThreadNum, int maxThreadNum); // 获取线程池对象
    bool addTask(std::function<void()> task);                             // 添加任务
    void waitAllTasksDone();                                              // 等待所有任务执行完毕

    /**
     * @brief 实现异步线程池的提交任务功能
     * 使用模板函数实现，可以接受任意类型的函数和参数
     * 因为使用了模版，所以需要后置返回值类型，编译器才能正确推导返回值类型
     * 使用&&进行完美转发，保证参数的类型不变（左值传左值，右值传右值）
     * 使用decltype(func(args...))获取函数的返回值类型
     * 使用future<decltype(func(args...))>作为返回值，可以获取函数的返回值
     * packged_task是一个函数对象，可以将函数包装起来，可以通过get_future()获取函数的返回值
     * 1.bind将函数和参数绑定在一起，实现无参调用，然后packaged_task包装bind返回的函数对象
     * 2.最后用lambda表达式封装packaged_task，转成function<void(void)>中，放入任务队列
     * task使用智能指针，是因为packaged_task是不可拷贝的。
     *
     * @tparam Func 函数类型
     * @tparam Args 参数类型，不定参数模板
     * @param func
     * @param args
     * @return future<decltype(func(args...))>
     */
    template <typename Func, typename... Args>
    auto submit(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
    {
        using returnType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<returnType> result = task->get_future();
        addTask([task]()
                { (*task)(); });
        return result;
    }
};