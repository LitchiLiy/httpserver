#if !defined(ASYNCLOGGING_H)
#define ASYNCLOGGING_H

/*
作用: 构建一个Buffer队列日志系统
*/


#include <string>
#include <mutex>
#include <condition_variable>
#include <thread.h>

#include <logStream.h>

#include <atomic>
#include <vector>


using namespace std;

class AsyncLogging {
public:
    /**
     * @brief 懒汉式创建一个同步异步日志系统
     *
     * @param basename
     * @param roolSize
     * @return AsyncLogging*
     */
    static AsyncLogging* getAsyncLog(const string& basename, off_t roolSize) {
        if (ALPtr == nullptr) {
            lock_guard<std::mutex> lock_(Mtx_);
            if (ALPtr == nullptr) {
                ALPtr = new AsyncLogging(basename, roolSize);
            }
        }
        return ALPtr;
    }

private:
    AsyncLogging(const string& basename, off_t rollSize, int flushInterval = 3);
    static std::mutex Mtx_;
    static AsyncLogging* ALPtr;
public:
    ~AsyncLogging() {
        if (isrunning) {
            stop();
        }
    }

    void append(const char* logline, size_t len);
    void start();
    void stop();


private:
    void threadFunc();


    typedef FixedBuffer<kLargeBuffer> tBuffer; // 一个4M的buffer
    typedef vector<unique_ptr<tBuffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;


private:
    Thread m_thread;
    mutex m_mutex;
    mutex m_cond_mutex;
    int cond_state = 0;
    condition_variable m_cond;
    bool isrunning;
    std::string basename; // 记录文件路径a/b/v/d/ssd.txt
    off_t rollSize;
    int flushInterval;


    BufferPtr currentBuffer; //都是unique的
    BufferPtr nextBuffer;
    BufferVector bufferVec;
};




#endif // ASYNCLOGGING_H
