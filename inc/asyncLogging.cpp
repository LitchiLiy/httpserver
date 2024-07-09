#include <asyncLogging.h>


#include <stdio.h>
#include <timestamp.h>
#include <logFile.h>
#include <string>
#include <logStream.h>



using namespace std;


AsyncLogging::AsyncLogging(const string& basename_, off_t rollSize, int flushInterval) :
    basename(basename_),
    rollSize(rollSize),
    flushInterval(flushInterval),
    currentBuffer(new tBuffer),
    nextBuffer(new tBuffer),
    isrunning(false),
    m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging") {
    // unique_lock<mutex> lock(m_cond_mutex);
    // m_cond.wait(m_cond_mutex);  // 初始化就给他锁住
    currentBuffer->bzero();
    nextBuffer->bzero();
    bufferVec.reserve(16);
}




void AsyncLogging::append(const char* logline, size_t len) {
    lock_guard<mutex> lock(m_mutex); // 锁住, 只有我一个线程能用
    if (currentBuffer->avail() > len)  // 当前的buffer能装下, 
    {
        currentBuffer->append(logline, len);
    }
    else {
        bufferVec.push_back(std::move(currentBuffer));

        if (nextBuffer) {
            currentBuffer = std::move(nextBuffer);
        }
        else {
            currentBuffer.reset(new tBuffer); // 新建一个buffer
        }
        currentBuffer->append(logline, len); // 装进去
        m_cond.notify_one(); // 唤醒线程的执行函数
    }
}

/**
 * @brief 当你创建了AsyncLogging对象, 调用start, 他会创建一个线程， 线程就会运行这个函数。
 *
 */
void AsyncLogging::threadFunc() {
    assert(isrunning == true);
    // countDown
    {
        lock_guard<mutex> lock(m_cond_mutex);
        cond_state = 1;
        m_cond.notify_all(); // 这里把start的wait函数唤醒
    }
    LogFile output(basename, rollSize, false); // 打开一个输出文件的fd
    BufferPtr newBuffer1(new tBuffer);
    BufferPtr newBuffer2(new tBuffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);  // 预留空间
    while (isrunning) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            unique_lock<mutex> lock(m_cond_mutex);
            if (bufferVec.empty()) {  // 不经常用
                m_cond.wait_for(lock, chrono::microseconds(flushInterval * 1000 * 1000)); // 阻塞等待notify， 单位秒
            }
            bufferVec.push_back(std::move(currentBuffer));  // 有数据进来后，转移给vec
            currentBuffer = std::move(newBuffer1);
            buffersToWrite.swap(bufferVec);
            if (!nextBuffer) {
                nextBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // 容量控制, 日志实在太多了没办法, 删一点.
        // 如果超过25个就打印一下, 并删除vec中的部分buffer. 主要是控制vec的大小
        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof buf, "timestamp is %ld second, %zd larger buffers\n",
                     Timestamp::now().showsec(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf))); // 将信息写入文件
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end()); // 删掉前两个
        }

        // 逐个写入到fd当中, 
        for (const auto& buffer : buffersToWrite) {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }

        // 写完之后, 调整大小位2
        if (buffersToWrite.size() > 2) {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        // 取出vec两个buffer, 以免新建浪费资源
        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        // 同上
        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        // 按道理此时应该buffer清空了, 为了保证清空, 直接重置
        buffersToWrite.clear();
        // 将缓冲区全部写进去
        output.flush();


    }
    // 最后再刷新一下缓冲区。
    output.flush();
}




void AsyncLogging::start() {
    isrunning = true;
    m_thread.createThread();

    {
        unique_lock<mutex> lock(m_cond_mutex);
        while (cond_state != 1) {
            m_cond.wait(lock); // 这里等待threadFunc的notify, wait的目的是确认创建了thread并执行之后再往下走.
        }
        cond_state = 0;
    }
}

void AsyncLogging::stop() {
    isrunning = false;
    cond_state = 1;
    m_cond.notify_all();
    m_thread.join();
}