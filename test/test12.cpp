
#include <logFile.h>
#include <logging.h>

/*
注意几点:
1. flush默认是flush输出stdout, 如果设置成logFIle的flush, 则flush的是FILE*
2. logFile是用来打开文件的

logging的使用流程就是
1. 设置输出函数, 设置flush函数
2. 调用LOGINFO


结果:20240702 08:12:45 tid=140737352702336 INFO  1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 9 - home/litchily/obj/Lning_doc/lMuduo/test12.cpp:44

哪里可以看到前面部分的内容? 在logging的Impl的构造函数里, 有对tid=pthread的构建, 也有时间的构建. 也就是说你调用LOG等宏的时候, 所有内容都是卸载Impl的m_stream中.
*/


#include <unistd.h>

std::unique_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

void flushFunc()
{
    g_logFile->flush();
}

int main(int argc, char* argv[])
{
    char name[256] = { '\0' };
    char arr[] = { "../log/logFile" };
    strncpy(name, arr, sizeof name - 1);
    g_logFile.reset(new LogFile(::basename(name), 200 * 1000));
    Logger::setOutput(outputFunc);
    Logger::setFlush(flushFunc);

    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 10; ++i)
    {
        LOG_INFO << line << i;

        usleep(1000);
    }
}
