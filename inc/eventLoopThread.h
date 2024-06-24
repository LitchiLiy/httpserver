#if !defined(EVENTLOOPTHREAD_H)
#define EVENTLOOPTHREAD_H


#include <memory>
#include <pthread.h>
#include <callBacks.h>
#include <string>
#include <mutex>
#include <condition_variable>

#include <thread.h>




class EventLoop;

class EventLoopThread {
public:
    EventLoopThread(const threadInitCallBack& cb = threadInitCallBack(), const std::string& name = std::string());
    ~EventLoopThread();


    EventLoop* startLoop();

private:
    void threadfunc();

private:



    // 关于线程初始化
    Thread m_thread;
    threadInitCallBack m_pthreadInitCallback;
    EventLoop* m_loop; // EventLoop不能用shared指着，因为里面的mutex不行


    // 条件变量和锁
    std::mutex m_mutex;
    std::condition_variable m_cond;

};


#endif // EVENTLOOPTHREAD_H)
