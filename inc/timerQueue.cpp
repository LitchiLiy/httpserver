#include <timerQueue.h>
#include <iostream>
#include <string.h>

#include <timer.h>
#include <eventLoop.h>
#include <timerId.h>
#include <unistd.h>
#include <assert.h>





// 初始化构件队列时, 构建了一个定时器, 一个触发这个定时器的channal
TimerQueue::TimerQueue(EventLoop* lp) : m_loop(lp), m_queueTimerfd(TimerQueue::createTimerfd()), m_timerfdChannel(lp, m_queueTimerfd) {
    m_timerfdChannel.setReadCallBack(std::bind(&TimerQueue::timerHandleRead, this));
    m_timerfdChannel.setReadEnable();
}




TimerQueue::~TimerQueue() {
    m_timerfdChannel.disableAll();
    m_timerfdChannel.remove();
    close(m_queueTimerfd);
    for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
        delete it->second;
    }
}

vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timers_.size() == m_activeTimerSet.size());
    vector<TimerQueue::Entry> expired;
    TimerQueue::Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX)); // 无符号整数最大值

    TimerList::iterator end = timers_.lower_bound(sentry); // set为升序排序, 返回第一个大于等于now的定时器的迭代器
    assert(end == timers_.end() || now < end->first);

    for (TimerList::iterator it = timers_.begin(); it != end; ++it) {
        expired.emplace_back(*it);
    }

    timers_.erase(timers_.begin(), end); // 将所有到期的定时器除掉
    for (const TimerQueue::Entry& it : expired) {
        actTimer timer(it.second, it.second->showSeq());
        size_t n = m_activeTimerSet.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == m_activeTimerSet.size());
    // 此时exp里面的timer指针次数为1, 而且zz不是一个悬空指针
    // auto zz = expired.begin()->second;
    return expired; // return 走到 } 之后, 所有的shptr变成空, 但是通过测试直到, 这样做时没有问题的啊
}


int TimerQueue::createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        std::cout << "Create time out" << std::endl;
    }
    return timerfd;
}

/**
 * @brief new一个定时器, 并且添加timerQueue的队列中set
 *
 * @param cb
 * @param when
 * @param interval
 * @return TimerId 这个ID里面保存着一个new timer的指针, 这个指针十分不安全, 因为类的析构函数会delete掉它, 从而不可用
 */
TimerId TimerQueue::addTimer(const callBack_f& cb, const Timestamp& when, double interval) {
    Timer* timer = new Timer(cb, when, interval);
    m_loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer)); // 让线程去调用这个定时器操作函数
    return TimerId(timer, timer->showSeq());
}


void TimerQueue::addTimerInLoop(Timer* timer) {
    // addTimer一定要在本线程调用才行. 
    m_loop->assertInLoopThread();
    bool earliestChanged = insertTimerSet(timer);
    // 判断是否为唯一一个插入的timer或者排在第一位
    if (earliestChanged) {
        resetTimerfd(m_queueTimerfd, timer->showExpired());
    }
}

bool TimerQueue::insertTimerSet(Timer* timer) {
    m_loop->assertInLoopThread();
    assert(m_activeTimerSet.size() == timers_.size());
    bool earliestChanged = false;
    // 这里有两个set, 一个是管理所有的定时器信息, 一个是act定时器
    Timestamp tmp = timer->showExpired();
    auto it = timers_.begin();
    if (tmp < it->first || it == timers_.end()) {
        earliestChanged = true;
    }

    // 最终, new 的timer指针被插入到这里
    timers_.insert(TimerQueue::Entry(tmp, timer));
    m_activeTimerSet.insert(TimerQueue::actTimer(timer, timer->showSeq()));

    assert(m_activeTimerSet.size() == timers_.size());
    return earliestChanged;
}



void TimerQueue::resetTimerfd(int timerfd, Timestamp expir) {
    // 重置定时器
    struct itimerspec newTimeSpec;
    struct itimerspec oldTimeSpec;

    memset(&newTimeSpec, 0, sizeof(newTimeSpec));
    memset(&oldTimeSpec, 0, sizeof(oldTimeSpec));

    newTimeSpec = calcuTstampToNow(expir);
    int ret = timerfd_settime(timerfd, 0, &newTimeSpec, &oldTimeSpec);
    if (ret < 0) {
        std::cout << "timerfd_settime error" << std::endl;
    }
}


itimerspec TimerQueue::calcuTstampToNow(Timestamp& exp) {
    // 先算微妙插值
    int64_t delta_us = exp.showusec() - Timestamp::now().showusec();
    if (delta_us < 100) { // 负数也算
        delta_us = 100;
    }
    itimerspec ts;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = delta_us / 1000000;
    ts.it_value.tv_nsec = (delta_us % 1000000) * 1000;
    return ts;
}


// timerQueue创建一个定时器fd, 一个专用Channel, 还有Channel专用的定时器触发函数
void TimerQueue::timerHandleRead() {
    // 必须在本线程执行, 这个函数, 说明定时器的触发后执行的任务全部都必须在本IO执行
    m_loop->assertInLoopThread();
    Timestamp now = Timestamp::now();
    uint64_t timeNum = 0;
    ssize_t ret = read(m_queueTimerfd, &timeNum, sizeof(timeNum)); // 不读出来就一直触发LT模式
    cout << "timer out read ：" << timeNum << endl;

    // 处理堆积的定时器, 取出来, 全部执行

    auto expvec = getExpired(now);

    callingExpiredTimers_ = true;
    m_cancelingTimerSet.clear();

    for (const Entry& it : expvec) {
        it.second->runTimerCallBack();
    }

    callingExpiredTimers_ = false;
    getExpAfterReset(expvec, now);  // 对应reset

}

void TimerQueue::getExpAfterReset(vector<TimerQueue::Entry> expVec, Timestamp now) {
    Timestamp timerfdNextSet;
    // 检查是否周期的办法在timer类实例里面
    for (const Entry& it : expVec) {
        actTimer timer(it.second, it.second->showSeq());
        if (it.second->isRepeat() && m_cancelingTimerSet.find(timer) == m_cancelingTimerSet.end()) { // 是否周期
            it.second->restart(now);  // 更改该timer的下一次触发时间戳
            insertTimerSet(it.second); // 刚被getExpired取出来的又重新加回去
        }
        else {
            // 非周期或者要被取消的 删除定时器Timer这个new
            delete it.second;
        }
    }

    // 判断下一个触发的事件stamp是什么. 更新timerfd_setting
    if (!timers_.empty()) {
        timerfdNextSet = timers_.begin()->second->showExpired(); // 这个变成了seq导致一直触发LT， 改成Exp就好了
    }

    if (timerfdNextSet.isVaild()) // 这个时间戳是否大于0, 小于零就是错误的时间错, 和now无关
    {
        resetTimerfd(m_queueTimerfd, timerfdNextSet); // 将这个时间timerfd_setting
    }
}

void TimerQueue::cancel(TimerId id) {
    m_loop->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, id));
}

/**
 * @brief 取消的原理是, 检查Timer地址和seq组成的pair, 在act里面, 则将他们推出去.
 *
 * @param id
 */
void TimerQueue::cancelInLoop(TimerId id) {
    m_loop->assertInLoopThread();
    assert(timers_.size() == m_activeTimerSet.size());
    actTimer timer(id.m_timer, id.m_seq);
    auto it = m_activeTimerSet.find(timer);
    // 确定能找到
    if (it != m_activeTimerSet.end()) {
        size_t n = timers_.erase(Entry(it->first->showExpired(), it->first));
        assert(n == 1);
        delete it->first;
        m_activeTimerSet.erase(it);
    }
    else if (callingExpiredTimers_) { // 如果正在执行处理内部的定时器, 那就先存起来待会再取消
        m_cancelingTimerSet.insert(timer);
    }
    // 此时已经没有那个了
    Timer* nowTimer = m_activeTimerSet.begin()->first;
    resetTimerfd(m_queueTimerfd, nowTimer->showExpired());
    assert(timers_.size() == m_activeTimerSet.size());
}
