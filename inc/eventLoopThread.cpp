#include <eventLoopThread.h>
#include <eventLoop.h>
#include <unistd.h>



typedef std::function<void()> callBack_f;
EventLoopThread::EventLoopThread(const threadInitCallBack& cb, const string& name) :m_thread(std::bind(&EventLoopThread::threadfunc, this), name) {
    m_pthreadInitCallback = cb;
    m_loop = nullptr;
}


EventLoopThread::~EventLoopThread() {

}


EventLoop* EventLoopThread::startLoop() {
    assert(!m_thread.isStarted());
    EventLoop* loop1 = nullptr;
    m_thread.createThread();
    {
        unique_lock<std::mutex> lg(m_mutex);
        while (m_loop == nullptr) {
            m_cond.wait(lg);
        }
        loop1 = m_loop;
    }
    return loop1;
}

void EventLoopThread::threadfunc() {  // 线程最终在Data中被InLoop中调用。
    EventLoop loop;


    if (m_pthreadInitCallback) {
        m_pthreadInitCallback(&loop); // 线程初始化函数
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_all();
    }
    m_loop->loop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}

