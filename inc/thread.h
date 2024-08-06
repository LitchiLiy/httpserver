#if !defined(THREAD_H)
#define THREAD_H

#include <callBacks.h>
#include <string>
#include <pthread.h>
#include <condition_variable>
#include <mutex>


class Thread {
public:
    Thread(callBack_f threadfunc, const std::string& name = std::string());
    ~Thread();

    void createThread();
    void join();
    bool isStarted() const { return isstarted; }
    bool isJoined() const { return isjoined; }
    static void* createThreadCb(void* arg);

    friend class ThreadData;
private:
    pthread_t m_tid;
    pthread_t m_pthreadfd;
    bool isjoined;
    bool isstarted;
    std::string m_name;
    callBack_f m_threadfunc;
    std::mutex m_mutex;
    std::condition_variable m_cond;

};



#endif // THREAD_H
