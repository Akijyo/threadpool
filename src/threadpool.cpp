#include "../include/threadpool.h"
using namespace std;

ThreadPool::ThreadPool(int minThreadNum, int maxThreadNum)
{
    cout << "线程池的构造函数开始" << endl;
    this->minThreadNum = minThreadNum;
    this->maxThreadNum = maxThreadNum;
    this->busyThreadNum = 0;
    this->isRun = true;
    for (int i = 0; i < minThreadNum; i++)
    {
        thread t(&ThreadPool::workThread, this);
        workerID[t.get_id()] = move(t);
        this->liveThreadNum++;
    }
    addThreadID = thread(&ThreadPool::addThread, this);
    destroyThreadID = thread(&ThreadPool::destroyThread, this);
    cout << "线程池的构造函数结束" << endl;
}

ThreadPool *ThreadPool::getThreadPool(int minThreadNum, int maxThreadNum)
{
    static ThreadPool pool(minThreadNum, maxThreadNum);
    return &pool;
}

void ThreadPool::workThread()
{
    while (isRun)
    {
        function<void(void)> task;
        { // 上锁的作用域
            unique_lock<mutex> lock(this->mtx);
            while (this->taskQueue.empty() && this->isRun) // 任务队列为空，则循环阻塞
            {
                this->consumerCond.wait(lock);
                if (this->exitThread) // 销毁管理者线程通知工作线程退出
                {
                    cout << "线程" << this_thread::get_id() << "退出" << endl;
                    this->liveThreadNum--; // 存活线程数减1

                    {
                        std::unique_lock<std::mutex> lock(this->exitMtx);
                        this->exitID.push_back(this_thread::get_id());
                    }

                    this->exitThread = false; // 重置退出标志
                    return;
                }
                if (!this->isRun) // 再次判断线程池是否关闭
                {
                    return;
                }
            }
            if (!this->isRun) // 再次判断线程池是否关闭
            {
                return;
            }
            task = this->taskQueue.front(); // 取出任务
            this->taskQueue.pop_front();
        }
        cout << "线程" << this_thread::get_id() << "开始执行任务" << endl;
        if (task) // 任务执行，这里不用加锁，同时busyThreadNum++，busyThreadNum--，是原子操作
        {
            this->busyThreadNum++;
            try
            {
                task();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            this->busyThreadNum--;
        }
        cout << "线程" << this_thread::get_id() << "任务执行结束" << endl;
    }
}

void ThreadPool::addThread()
{
    while (this->isRun)
    {
        this_thread::sleep_for(chrono::seconds(1)); // 每隔1s检查一次
        unique_lock<mutex> lock(this->mtx);
        int count = (this->taskQueue.size() - this->liveThreadNum) * 2; // 任务队列中任务数减去存活线程数的两倍
        // 任务队列中任务数大于存活线程数，并且存活线程数小于最大线程数,则循环地不断增加工作线程
        while (this->isRun && this->taskQueue.size() > this->liveThreadNum && this->liveThreadNum < this->maxThreadNum && count > 0)
        {
            thread t(&ThreadPool::workThread, this);
            workerID[t.get_id()] = move(t);
            this->liveThreadNum++;
            count--;
            cout << "线程" << t.get_id() << "创建成功(线程增加函数)" << endl;
        }
    }
}

void ThreadPool::destroyThread()
{
    while (this->isRun)
    {
        this_thread::sleep_for(chrono::seconds(1)); // 每隔1s检查一次

        unique_lock<mutex> lock(this->mtx);

        {
            std::unique_lock<std::mutex> lock(this->exitMtx);
            for (auto &id : exitID)
            {
                if (this->workerID[id].joinable())
                {
                    this->workerID[id].join();
                }
                this->workerID.erase(id);
                cout << "线程" << id << "销毁成功(线程销毁函数)" << endl;
            }
            exitID.clear();
        }

        // 任务队列中任务数小于存活线程数，并且存活线程数大于最小线程数,销毁工作线程
        if (this->isRun && this->liveThreadNum > this->minThreadNum && this->busyThreadNum < this->liveThreadNum * 2)
        {
            this->exitThread = true; // 通知工作线程退出
            this->consumerCond.notify_one();
        }
    }
}

bool ThreadPool::addTask(function<void()> task)
{
    if (!this->isRun)
    {
        cout << "线程池已经关闭,无法添加任务" << endl;
        return false;
    }
    unique_lock<mutex> lock(this->mtx); // 保护taskQueue
    this->taskQueue.push_back(task);
    this->consumerCond.notify_one(); // 通知一个消费者线程
    return true;
}

void ThreadPool::waitAllTasksDone()
{
    while (true)
    {
        this->mtx.lock();
        if (this->taskQueue.empty() && this->busyThreadNum == 0)
        {
            break; // 任务队列为空且没有忙线程，退出循环
        }
        this->mtx.unlock();
        this_thread::sleep_for(chrono::seconds(1)); // 等待1s后重新检查
    }
    cout << "所有任务已完成" << endl;
}

ThreadPool::~ThreadPool()
{
    cout << "线程池的析构函数开始" << endl;
    this->isRun = false;
    this->consumerCond.notify_all();
    this->addThreadID.join();
    this->destroyThreadID.join();
    for (auto &t : this->workerID)
    {
        if (t.second.joinable())
        {
            t.second.join();
        }
    }
    cout << "线程池已经关闭" << endl;
}