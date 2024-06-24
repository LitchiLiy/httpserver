#include <eventLoop.h>
#include <unistd.h>
#include <utility>
#include <sys/eventfd.h>
#include <epoller.h>
#include <channel.h>
#include <timerQueue.h>
#include <callBacks.h>


// // 一个静态指针, 用来指向运行当前函数的线程, 没啥用
// EventLoop* t_loopInThisThread;

EventLoop::EventLoop() :threadId_(pthread_self()), v_pendingFunctors(std::vector<callBack_f>{}) {

    looping_ = false;
    isquit = false;
    std::cout << "EventLoop created"
        << this
        << " in thread "
        << threadId_
        << std::endl;

    // t_loopInThisThread = this;
    p_Epoller = std::make_shared<Epoller>(this);

    m_wakeupFd = createEventfd();
    sp_wakeupChannel = std::make_shared<Channel>(this, m_wakeupFd);
    sp_wakeupChannel->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    sp_wakeupChannel->setReadEnable();
    m_timerQueue = std::make_shared<TimerQueue>(this); // 定时器一个channel, wakeup一个channel


}

EventLoop::~EventLoop() {
    assert(!looping_);
    // t_loopInThisThread = nullptr;
}

bool EventLoop::isInLoopThread() {
    return threadId_ == pthread_self();
}

void EventLoop::aboutNotInLoopThread() {
    // 如果执行loop的函数不在执行EventLoop的线程当中
    std::cout << "Not in the Thread" << std::endl;
}

void EventLoop::assertInLoopThread() {
    if (!isInLoopThread()) {
        aboutNotInLoopThread();
    }
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();

    looping_ = true;
    isquit = false;
    while (!isquit) {
        v_actChannels.clear();
        p_Epoller->epolling(-1, &v_actChannels);
        isEventHandling = true;
        for (Channel* ch : v_actChannels) {
            currentActiveChannel_ = ch;
            ch->handleEvent();
        }
        currentActiveChannel_ = nullptr;
        isEventHandling = false;
        doPendingFunctors(); // doPend执行的是另个一channel的vec, 所以上面的clear影响不到我们.
    }
    std::cout << "EventLoop" << this << " stoped Looping" << std::endl;
    looping_ = false;

}

void EventLoop::quit() {
    isquit = true;
    if (!isInLoopThread()) {
        wakeup(); // 为什么本线程就不用唤醒? 因为这个quit一定是一个channel来调用的, 此时应该在epoll之内,一定会有新循环, 而外部调用可能阻塞在epoll中.
    }
}

void EventLoop::updateChannel(Channel* ch) {
    assert(ch->ownerloop() == this);
    assertInLoopThread();
    p_Epoller->updateChannel(ch);
}

void EventLoop::runInLoop(callBack_f cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(callBack_f cb) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        v_pendingFunctors.push_back(std::move(cb));


    }

    if (!isInLoopThread() || m_callingPendingFunctors) {
        wakeup(); // 如果不是当前线程, 则唤醒. 因为有可能他阻塞在epoll中, actchannel不为空都不知道. 如果现在正在执行Pendingfunction的任务, 也唤醒, 这样在执行结束之后就可以再次执行了. 这两个都保证了cb能被及时调用.
    }
}

void EventLoop::doPendingFunctors() {
    // 交换的目的是为了防止此时有人往factor里面加东西, 而阻塞的太久了,因为如果阻塞来运行可能要运行好久.
    std::vector<callBack_f> functors;
    m_callingPendingFunctors = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(v_pendingFunctors);
    }
    for (auto& f : functors) {
        f();
    }
    m_callingPendingFunctors = false;
}


void EventLoop::wakeup() {
    eventfd_t one = 0xFFFF;
    auto n = eventfd_write(m_wakeupFd, one); // 如果成功输入那就是0?, 反正这里总是返回0, 确实没问题, 测试过read那边读出来是65536, 就是0xFFFF
    if (n < 0) {
        std::cout << "wakeup error" << std::endl;
    }
}

void EventLoop::handleRead() {
    // 只用来唤醒
    uint64_t one = 1;
    ssize_t n = read(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        std::cout << "wakeup error" << std::endl;
    }
}

int EventLoop::createEventfd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        std::cout << "create eventfd error" << std::endl;
    }
    return fd;
}

TimerId EventLoop::runAt(const Timestamp& when, const callBack_f& cb) {
    return m_timerQueue->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runAfter(double delay, const callBack_f& cb) {
    int64_t delta_us = Timestamp::now().showusec() + static_cast<int64_t>(delay * 1000 * 1000);
    Timestamp when = Timestamp(delta_us);
    return m_timerQueue->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runEvery(double interval, const callBack_f& cb) {
    int64_t delta_us = Timestamp::now().showusec() + static_cast<int64_t>(interval * 1000 * 1000);
    Timestamp when = Timestamp(delta_us);
    return m_timerQueue->addTimer(cb, when, interval);
}

// 移除的时候需要注意, 正在处理的channel是不是这个, 等待处理的act中有没有这个.
void EventLoop::remove(Channel* ch) {
    assert(ch->ownerloop() == this);
    assertInLoopThread();
    if (isEventHandling)
    {
        assert(currentActiveChannel_ == ch ||
            std::find(v_actChannels.begin(), v_actChannels.end(), ch) == v_actChannels.end());
    }
    p_Epoller->removeChannel(ch);
}