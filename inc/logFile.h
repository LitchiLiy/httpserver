#if !defined(LOGFILE_H)
#define LOGFILE_H

#include <memory>
#include <mutex>
#include <string>
#include <fstream>
using namespace std;

// 作用就是保存本地txt文件的fd, 然后将string信息写入文件, 还有冲刷的操作, 这就是基层类., z
class AppendFile
{
public:
    /**
     * @brief
     *
     * @param filename 这个filename是一个路径名字， 比如: /home/litchily/log.txt
     */
    explicit AppendFile(const std::string& filename);
    ~AppendFile();
    void append(const char* logline, size_t len);
    void flush();
    off_t writtenBytes() const { return writtenBytes_; }
private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};


/*
作用: 调动appendFile已经继承的功能, 然后实现日志的滚动更替(换名字等)
*/

class LogFile
{
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,    // 冲刷周期为3
            int checkEveryN = 1024);  // 检查count阈值为1024
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    static string getLogFileName(const string& basename, time_t* now);

    const string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    mutex m_mutex;
    time_t startOfPeriod_;
    time_t lastRoll_;  // 上一次滚动的事件
    time_t lastFlush_; // 上一次刷新的事件
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60 * 60 * 24;
};




#endif // LOGFILE_H)
