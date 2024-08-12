#include <eventLoopThreadPool.h>


#include <eventLoop.h>
#include <eventLoopThread.h>
#include <vector>



using namespace std;
EventLoopThreadPool* EventLoopThreadPool::ELTPtr = nullptr;
std::mutex EventLoopThreadPool::Mtx_;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* el, const string& nameArg) :
    m_fatherLoop(el),
    m_name(nameArg),
    m_threadNum(0),
    m_nextThread(0),
    isstarted(false) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    // 线程池声明由tcpserver来掌管.
    ELTPtr = nullptr;

}

void EventLoopThreadPool::startPool(const threadInitCallBack& cb) {
    assert(!isstarted);
    m_fatherLoop->assertInLoopThread();
    isstarted = true;


    for (int i = 0; i < m_threadNum; ++i) {
        char buf[m_name.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", m_name.c_str(), i);
        EventLoopThread* thread = new EventLoopThread(cb, buf); // 

        m_threadVec.emplace_back(thread); // 这里用一个sharedptr的数组来管理new
        m_loopVec.emplace_back(thread->startLoop());
    }


    // 如果线程池没有规定线程数量, 那么就直接用当前线程执行
    if (m_threadNum == 0 && cb) {
        cb(m_fatherLoop);
    }
}


EventLoop* EventLoopThreadPool::getNextLoop() {
    m_fatherLoop->assertInLoopThread();
    assert(isstarted);
    EventLoop* tmpLoop = m_fatherLoop;

    if (!m_loopVec.empty()) {
        tmpLoop = m_loopVec[m_nextThread];
        m_nextThread++;
        if (m_nextThread >= m_loopVec.size()) {
            m_nextThread = 0;
        }
    }
    // 如果池子本来就没有线程, 那么就把当前主线程返回回去
    return tmpLoop;
}



// 如果池子有Event, 那么就不返回父亲线程, 不然返回父亲线程
vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    m_fatherLoop->assertInLoopThread();
    assert(isstarted);
    if (m_loopVec.empty()) {
        return std::vector<EventLoop*>(1, m_fatherLoop);
    }
    else {
        return m_loopVec;
    }
}

// 给一个hash码, 返回对应hash码的放在池子里面的一个EL指针
EventLoop* EventLoopThreadPool::getLoopForHash(uint64_t hash) {
    m_fatherLoop->assertInLoopThread();
    assert(isstarted);
    if (m_loopVec.empty()) {
        return m_fatherLoop;
    }
    else {
        return m_loopVec[hash % m_loopVec.size()];
    }
}

