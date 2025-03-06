#include"./include/threadpool.h"
using namespace std;

void working()
{
    int number=10;
    while (number)
    {
        this_thread::sleep_for(chrono::seconds(1));
        cout<<"线程"<<this_thread::get_id()<<"的输出："<<number<<endl;
        number--;
    }
}

int calculate(int a,int b)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a+b;
}

int main()
{
    ThreadPool *pool=ThreadPool::getThreadPool(5,10);
    vector<future<int>> res;
    for (int i = 0; i < 20; i++)
    {
        res.emplace_back(pool->submit(calculate,i,i*2));
        this_thread::sleep_for(chrono::microseconds(400));
    }
    for(auto &it:res)
    {
        cout<<"计算结果："<<it.get()<<endl;
    }
    this_thread::sleep_for(chrono::seconds(1000000));
    pool->waitAllTasksDone();
    return 0;
}