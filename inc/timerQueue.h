#if !defined(TIMERQUEUE_H)
#define TIMERQUEUE_H



#include <set>
#include <vector>
#include <sys/timerfd.h>
#include <callBacks.h>
#include <memory>

/*
这里保存着一个定时器fd, 因此只需要在这里保存着定时器信息就可以了, 启动这个实例可以自动实现定时器周期与清楚.

对于Timer信息, 我们保证其声明周期与actTimerset的声明周期是一致的就行.

即使定时器信息里面的周期是true, 只要我不让他参与定时器, 他就不会自己参与周期, 即一旦被我踢出周期, 则永远不能自动回来.
*/



#include <timestamp.h>
#include <channel.h>


class Timer;
class TimerId;   // 作为函数的形参和输出同样不需要包括头文件
class EventLoop;

using namespace std;

class TimerQueue {
public:
    TimerQueue(EventLoop* lp);
    ~TimerQueue();

    TimerId addTimer(const callBack_f& cb, const Timestamp& when, double interval);
    void cancel(TimerId id);
    void cancelInLoop(TimerId id);

private:

    typedef std::pair<Timestamp, Timer*> Entry;  // 通过时间戳来排序, 来找对应的定时器信息
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> actTimer;
    typedef std::set<actTimer> ActiveTimerSet;

    void timerHandleRead(); // 放到Channel中的, 供eventLoop调用, 当定时器触发时, 最终调用这个
    // void reset(const std::vector<Entry>& expired, Timestamp now);

    vector<TimerQueue::Entry> getExpired(Timestamp now);  // 获得到期的定时器数组.



    int createTimerfd(); // 初始化用的
    void addTimerInLoop(Timer* timer); // 一个回调函数, 用来往本类中添加定时器
    bool insertTimerSet(Timer* timer);  // 把定时器信息插入到两个set中
    void resetTimerfd(int timerfd, Timestamp expir);  // 通过fd设置linux的定时器
    itimerspec calcuTstampToNow(Timestamp& exp); // 用来计算时间戳还有多久触发, 返回ts结构体

    // handle期间的函数
    void getExpAfterReset(vector<TimerQueue::Entry> expVec, Timestamp now);  // 触发的定时器事件处理完之后, 检查这些定时器是否周期, 是否被取消, 来决定重新timerfd_setting

private:
    EventLoop* m_loop;
    const int m_queueTimerfd;
    Channel m_timerfdChannel;

    TimerList timers_;  // 已排序的定时器列表, 按timestamp排
    ActiveTimerSet m_activeTimerSet;
    ActiveTimerSet m_cancelingTimerSet;  // 放在内部的定时器不用特定去清除他, 在周期判断中如果能在这个set中发现, 则不让他参与周期就行.

    bool callingExpiredTimers_ = false; // 用在Handle中, 表示正在处理getExpir取出来的所有Entry的回调函数中
};


#endif // TIMERQUEUE_H






