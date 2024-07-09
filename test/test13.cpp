
#include <logging.h>
#include <logFile.h>
#include <pthread.h>


#include <memory>
#include <stdio.h>
#include <unistd.h>

/*
测试logging的各种东西, 至少INFO, ERRNO, LOG_WARN都是能用的

还有一点是, logging.h里保管者一个全局output函数指针, 只有一个
*/




int g_total;
FILE* g_file;
std::unique_ptr<LogFile> g_logFile;

void dummyOutput(const char* msg, int len)
{
    g_total += len;
    if (g_file)
    {
        fwrite(msg, 1, len, g_file);
    }
    else if (g_logFile)
    {
        g_logFile->append(msg, len);
    }
}

// 压力测试
void bench(const char* type)
{
    Logger::setOutput(dummyOutput); // 存入到文件中
    Timestamp start(Timestamp::now());
    g_total = 0;

    int n = 1000 * 1000;
    const bool kLongLog = false;
    string empty = " ";
    string longStr(3000, 'X');
    longStr += " ";
    for (int i = 0; i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
            << (kLongLog ? longStr : empty)
            << i;
    }
    Timestamp end(Timestamp::now());
    double seconds = timeDifference(end, start);
    printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
        type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

void* logInThread(void* arg)
{
    LOG_INFO << "logInThread";
    usleep(1000);
    return nullptr;
}

int main()
{
    getppid(); // for ltrace and strace

    // ThreadPool pool("pool");
    // pool.start(5);
    // pool.run(logInThread);
    // pool.run(logInThread);
    // pool.run(logInThread);
    // pool.run(logInThread);
    // pool.run(logInThread);

    pthread_t a1;
    pthread_t a2;
    pthread_t a3;
    pthread_t a4;
    pthread_t a5;

    {
        // 默认输出是std
        pthread_create(&a1, nullptr, logInThread, nullptr);
        pthread_create(&a2, nullptr, logInThread, nullptr);
        pthread_create(&a3, nullptr, logInThread, nullptr);
        pthread_create(&a4, nullptr, logInThread, nullptr);
        pthread_create(&a5, nullptr, logInThread, nullptr);

        LOG_TRACE << "trace";
        LOG_DEBUG << "debug";
        LOG_INFO << "Hello";
        LOG_WARN << "World";
        LOG_ERROR << "Error";
        LOG_INFO << sizeof(Logger);
        LOG_INFO << sizeof(LogStream);
        LOG_INFO << sizeof(Fmt);
        LOG_INFO << sizeof(LogStream::Buffer);
    }
    sleep(1);
    // bench("nop");

    char buffer[64 * 1024];

    g_file = fopen("../log/LogFile", "w");
    setbuffer(g_file, buffer, sizeof buffer);
    bench("../log/LogFile");  // 生成到相对地址的目的地log中了, 成功
    fclose(g_file);

    // g_file = fopen("./tmp/log", "w");
    // setbuffer(g_file, buffer, sizeof buffer);
    // bench("/tmp/log");
    // fclose(g_file);

    // g_file = NULL;
    // g_logFile.reset(new LogFile("test_log_st", 500 * 1000 * 1000, false));
    // bench("test_log_st");

    // g_logFile.reset(new LogFile("test_log_mt", 500 * 1000 * 1000, true));
    // bench("test_log_mt");
    // g_logFile.reset();

    return 0;
}
