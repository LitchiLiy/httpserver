#include <eventLoop.h>
#include <unistd.h>
#include <utility>
#include <sys/eventfd.h>
#include <epoller.h>
#include <channel.h>
#include <timerQueue.h>
#include <callBacks.h>
#include <logging.h>
#include <sys/socket.h>
#include <string.h>
#include <selectPoll.h>
#include <pollbase.h>





// // 一个静态指针, 用来指向运行当前函数的线程, 没啥用
// EventLoop* t_loopInThisThread;

EventLoop::EventLoop(const string pollmode) :threadId_(pthread_self()), v_pendingFunctors(std::vector<callBack_f>{}) {
    looping_ = false;
    isquit = false;
    // std::cout << "EventLoop created"
    //     << this
    //     << " in thread "
    //     << threadId_
    //     << std::endl;
    // t_loopInThisThread = this;
    if (pollmode == "epoll") {
        m_pbasePtr = shared_ptr<Pollbase>(new Epoller(this));
    }
    else if (pollmode == "select") {
        m_pbasePtr = shared_ptr<Pollbase>(new SelectPoll(this));
    }
    else {
        LOG_ERROR << "pollmode is not right, please check it";
    }
    // m_pbasePtr = shared_ptr<Pollbase>(new Epoller(this));
    // m_pbasePtr = shared_ptr<Pollbase>(new SelectPoll(this));
    m_wakeupFd = createEventfd();
    sp_wakeupChannel = std::make_shared<Channel>(this, m_wakeupFd);
    sp_wakeupChannel->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    sp_wakeupChannel->setReadEnable();
    m_timerQueue = std::make_shared<TimerQueue>(this); // 定时器一个channel, wakeup一个channel

    LOG_INFO << "EventLoop created and address= " << this << ", tid= " << pthread_self() << ", pid= " << getpid();
    // LOG_INFO << "epollfd= " << m_pbasePtr->getEpollfd() << ", wakeupfd= " << m_wakeupFd;

}

EventLoop::~EventLoop() {
    assert(!looping_);
    // t_loopInThisThread = nullptr;
    LOG_INFO << "EventLoop= " << this << ", tid= " << pthread_self() << ", pid= " << getpid() << " destroyed";
    sp_wakeupChannel->disableAll();
    sp_wakeupChannel->remove();
    close(m_wakeupFd);

    sp_quitChannel->disableAll();
    sp_quitChannel->remove();



}

bool EventLoop::isInLoopThread() {
    return threadId_ == pthread_self();
}

void EventLoop::aboutNotInLoopThread() {
    // 如果执行loop的函数不在执行EventLoop的线程当中
    std::cout << "Not in the EL's Thread" << "Now Thread's Tid is: " << pthread_self() << " Pid is : " << getpid() << "But EL's Tid is: " << threadId_ << endl;
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
        Timestamp recTime = m_pbasePtr->poll(-1, &v_actChannels);
        isEventHandling = true;
        for (Channel* ch : v_actChannels) {
            currentActiveChannel_ = ch;
            ch->handleEvent(recTime);
        }
        currentActiveChannel_ = nullptr;
        isEventHandling = false;
        doPendingFunctors(); // doPend执行的是另个一channel的vec, 所以上面的clear影响不到我们.
    }
    LOG_INFO << "EventLoop= " << this << " loop exit";
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
    m_pbasePtr->updateChannel(ch);
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
        LOG_ERROR << "wakeup error";
    }
}

void EventLoop::handleRead() {
    // 只用来唤醒
    uint64_t one = 1;
    ssize_t n = read(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

int EventLoop::createEventfd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        LOG_ERROR << "create eventfd error";
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
    if (isEventHandling) {
        assert(currentActiveChannel_ == ch ||
               std::find(v_actChannels.begin(), v_actChannels.end(), ch) == v_actChannels.end());
    }
    m_pbasePtr->removeChannel(ch);
}

bool EventLoop::hasChannel(Channel* ch) {
    assert(ch->ownerloop() == this);
    assertInLoopThread();
    return m_pbasePtr->findChannel(ch);
}


void EventLoop::setMainEventLoop() {
    // 一个退出机制. 识别终端的输入
    sp_quitChannel = std::make_shared<Channel>(this, STDIN_FILENO);
    sp_quitChannel->setReadCallBack(std::bind(&EventLoop::handleQuit, this, std::placeholders::_1));
    sp_quitChannel->setReadEnable();
    main_EventLoop = true;
}

void EventLoop::handleQuit(Timestamp now) {

    char buf[1024];
    ssize_t n = read(0, buf, sizeof(buf));
    if (n > 0) {
        if (strncmp(buf, "quit\n", 5) == 0) {
            // std::cout << "EventLoop is quiting!" << std::endl;
            runInLoop(std::bind(&EventLoop::quit, this));
        }
    }

}