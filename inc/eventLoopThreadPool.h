#if !defined(EVENTLOOPTHREADPOOL_H) 
#define EVENTLOOPTHREADPOOL_H
/*
功能: 提供一个父EventLoop, 然后提供一个名字给你, 就完事了.
1. 初始化多个EventLoopThread, 用vector来掌管, 数量可以用set函数来设置
2. 一个start函数
*/

#include <vector>
#include <memory>
#include <string>

using namespace std;

#include <callBacks.h>

class EventLoop;
class EventLoopThread;



class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* el, const string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { m_threadNum = numThreads; };
    void startPool(const threadInitCallBack& cb = threadInitCallBack());

    EventLoop* getNextLoop();
    EventLoop* getLoopForHash(uint64_t hash);

    vector<EventLoop*> getAllLoops();

    bool isStarted() const { return isstarted; }
    const string& name() const { return m_name; }


private:
    EventLoop* m_fatherLoop;
    string m_name;
    bool isstarted;
    int m_threadNum;
    int m_nextThread;
    vector<shared_ptr<EventLoopThread>> m_threadVec;
    vector<EventLoop*> m_loopVec;

};



#endif // EVENTLOOPTHREADPOOL_H)    
