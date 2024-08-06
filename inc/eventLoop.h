/*
    说明
1. 一个线程只能调用一个eventLoop循环

功能
1. 调用Poller类来检测epoll, 让Poller把监听到的事件全部装到一个vec里面去.
2. 调用vec里面的channel, 调用每个channel的回调函数.

eventLoop结构
1. 公有函数
    1. 构造函数, 析构函数,
    2. Loop函数开始循环.
    3. setStop 停止循环
2. 私有函数
   1. is本线程? loop函数只能在实例所在的本线程调用, 每次调用loop都要判断是否本线程.
3. 私有成员
    1. Poller指针, 用来调用Poller函数
    2. isStop变量, 确认循环是否关闭
    3. 本线程的Pid
    4. isloop确认是否正在循环
*/
#if !defined(EVENTLOOP_H)
#define EVENTLOOP_H

#include <pollbase.h>
#include <pthread.h>
#include <iostream>
#include <assert.h>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>
#include <timerId.h>
#include <timestamp.h>
#include <logging.h>



// 都是以指针的形式存在
class TimerQueue;
class Epoller;
class Channel;


using namespace std;

typedef std::function<void()> callBack_f;
using cbf = void (*)();


class EventLoop {
public:
    typedef std::function<void()> callBack_f;
    EventLoop(const string pollMode);
    ~EventLoop();

    void loop();
    void assertInLoopThread(); // 用来发状态
    bool isInLoopThread();  // 用来确认状态

    void updateChannel(Channel* ch);
    void quit();

    // runInLoop
    void runInLoop(callBack_f cb); // 任何线程想做点什么事情都可以放到这个函数这里来做, 循环回调用回调执行, 这个就是脱离channel之外的一个线程回调实现方法, 或者说epoll就用channel, 不是epoll就用runInLoop.
    void wakeup();

    // addTimer构建
    TimerId runAt(const Timestamp& when, const callBack_f& cb);
    TimerId runAfter(double delay, const callBack_f& cb);
    TimerId runEvery(double interval, const callBack_f& cb);

    void remove(Channel* ch);
    void queueInLoop(callBack_f cb);

    vector<callBack_f> v_pendingFunctors{};

    // 确认channel移除
    bool hasChannel(Channel* ch);
    void setMainEventLoop();

    // 退出日志


private:
    void aboutNotInLoopThread();
    bool looping_;
    const pthread_t threadId_;
    bool isquit;
    std::shared_ptr<Pollbase> m_pbasePtr;
    std::vector<Channel*> v_actChannels;

    // 新增runInLoop

    void handleRead();
    void doPendingFunctors(); // 放在正在执行的任务之后, 在loop函数最后执行

    // 退出机制
    void handleQuit(Timestamp now);
    // 添加timer

    bool m_callingPendingFunctors;
    int m_wakeupFd;
    std::shared_ptr<Channel> sp_wakeupChannel; // 只能自己使用

    std::mutex m_mutex;


    // 编写wakeup
    int createEventfd();

    // 编写addtimer
    std::shared_ptr<TimerQueue> m_timerQueue;

    // 编写accept
    bool isEventHandling;
    Channel* currentActiveChannel_;

    // 终端输入退出机制
    std::shared_ptr<Channel> sp_quitChannel;
    bool main_EventLoop = false;

};


#endif // EVENTLOOP_H



