#include <thread.h>
#include <unistd.h>



Thread::Thread(callBack_f threadfunc, const std::string& name) {
    m_tid = 0;
    m_pthreadfd = 0;
    m_name = name;
    m_threadfunc = threadfunc;
    isstarted = false;
    isjoined = false;
}


void* Thread::createThreadCb(void* arg) {
    Thread* tt = static_cast<Thread*>(arg);
    tt->m_tid = pthread_self();
    {
        std::lock_guard<std::mutex> lg(tt->m_mutex);
        tt->m_cond.notify_all();
        tt->isstarted = true;
    }
    tt->m_threadfunc();
    return nullptr;
}

Thread::~Thread() {

}

void Thread::createThread() {

    int ret = pthread_create(&m_pthreadfd, nullptr, createThreadCb, this);
    // if (m_pthreadfd != 0) {
    //     isstarted = false;
    // }
    std::unique_lock<std::mutex> lock(m_mutex); // unique就是配合条件变量而使用的, 等条件变量使用wait的时候, 就会暂时释放锁的所有权.
    while (isStarted() != true) // 如果线程比本线程快, 则不会进入wait的环节.
    {
        m_cond.wait(lock);
    }

}

void Thread::join() {

    if (m_pthreadfd != 0) {
        isjoined = true;
        pthread_join(m_pthreadfd, nullptr);
    }
}
