/*
测试AsyncLogging和Logging的基本用法, 其实只测了INFO,关键用法
1. 将Logging的setOutput设置成AsyncLogging的append
2. 构建Async实例
3. 调动Async的start.

当使用INFO的一些东西的时候, Logging就会调用Async的函数. 这个文件至少证明, INFO是可以用的


*/

#include <asyncLogging.h>
#include <logging.h>
#include <timestamp.h>

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>






off_t kRollSize = 500 * 1000 * 1000;   // 设定roll大小位500MB

AsyncLogging* g_asyncLog = NULL; // 同步类实例指针

// 实际调用这个函数的是Logger的析构函数, 因为宏定义会构建Logger, 走到下一行就析构掉了, 就会调用这个函数.
void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}



void bench(bool longLog)  // bool 监测是否压力测试的标志
{
    Logger::setOutput(asyncOutput); // 设置输出函数供logging类实例调用.

    int cnt = 0; // 用来计数
    const int kBatch = 1000; // 一次性写入kBatch个日志
    string empty = " ";
    string longStr(3000, 'X'); // 3000个X
    longStr += " "; // 3001个

    for (int t = 0; t < 30; ++t)
    {
        Timestamp start = Timestamp::now();
        for (int i = 0; i < kBatch; ++i)
        {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                << (longLog ? longStr : empty)
                << cnt;
            ++cnt;
        }
        Timestamp end = Timestamp::now();
        printf("%f\n", timeDifference(end, start) * 1000000 / kBatch); // 时间差, 秒
        struct timespec ts = { 0, 500 * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char* argv[])
{
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = { 2 * kOneGB, 2 * kOneGB };
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());

    // char name[256] = { '\0' };  // 文件名
    char name[256] = { '\0' };  // 文件名
    char arr[] = { "./Logfile" };   // 直接本地文件名

    strncpy(name, arr, sizeof name - 1);


    AsyncLogging log(::basename(name), kRollSize);
    log.start();  // 构建一个新线程, 不管主线程的事情


    g_asyncLog = &log;

    bool longLog = false;


    bench(longLog);  // 主线程开始写日志
}
