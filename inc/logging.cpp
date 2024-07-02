#include <logging.h>

#include <sys/timerfd.h>
#include <pthread.h>
#include <stdlib.h>
#include <logStream.h>

#include <ctime>
#include <stdio.h>
#include <string>
using namespace std;

// __thread表示线程局部变量, 每个线程都会有这样的变量而不会共享
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

// 从错误中读取消息, 写入提供的缓冲区中.
const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

// 获取环境变量的值, 然后将其转换为LogLevel输出出去.
Logger::LogLevel initLogLevel()
{
    if (::getenv("MUDUO_LOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("MUDUO_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}


// 直接定义, 然后在main中调用
Logger::LogLevel g_logLevel = initLogLevel();

// 一个数组, 定义类型`
const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};


// 用来警示str是否与所记录的长度一致
// class T {
// public:
//     T(const char* str, unsigned len) :
//         m_str(str),
//         m_len(len) {
//         assert(m_len == strlen(m_str));
//     }


//     const char* m_str;
//     const unsigned m_len;
// }

// // 一般再类内LogStream的位置有一个this, 但是此时在类外, 所以我们要指定 << 的左边是谁.
// inline LogStream& operator<<(LogStream& s, T v) {
//     s.append(v.m_str, v.m_len);
//     return s;
// }
// 这里总出错, 
class T
{
public:
    T(const char* str, unsigned len)
        :str_(str),
        len_(len)
    {
        assert(strlen(str) == len_);
    }

    const char* str_;
    const unsigned len_;
};
// 一般再类内LogStream的位置有一个this, 但是此时在类外, 所以我们要指定 << 的左边是谁.
inline LogStream& operator<<(LogStream& s, T v)
{
    s.append(v.str_, v.len_);
    return s;
}

inline LogStream& operator << (LogStream& s, const Logger::SourceFile& v) {
    s.append(v.data(), v.size());
    return s;
}

void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);

}

void defaultFlush() {
    fflush(stdout);
}

// 定义两个全局变量
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;


Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile& file, int line) :
    m_time(Timestamp::now()),
    m_stream(),
    m_level(level),
    m_line(line),
    m_file(file) {
    // 时间, pid, pid大小三件套
    formatTime();
    m_stream << T((string(" tid=") + to_string(pthread_self()) + " ").c_str(), to_string(pthread_self()).size() + 6);
    m_stream << T(LogLevelName[level], 6);
    if (old_errno != 0) {
        m_stream << strerror_tl(old_errno) << " (errno=)" << old_errno << ") ";
    }
}

void Logger::Impl::formatTime() {
    int64_t uSec = m_time.showusec();
    time_t secPort = m_time.showsec();
    int usecPort = static_cast<int>(uSec % 1000000);
    if (secPort != t_lastSecond) {
        t_lastSecond = secPort;
        std::time_t now = std::time(nullptr);
        std::tm localtime;
        localtime_r(&now, &localtime);   // 获取东八区的时间
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
            localtime.tm_year + 1900, localtime.tm_mon + 1, localtime.tm_mday,
            localtime.tm_hour, localtime.tm_min, localtime.tm_sec);
        assert(len == 17);

    }
    // // 按UTC显示
    // Fmt us(".%06dZ ", usecPort);
    // assert(us.length() == 9);
    // m_stream << T(t_time, 17) << T(us.data(), 9);
    m_stream << T(t_time, 17);
}


void Logger::Impl::finish() {
    m_stream << " - " << m_file.m_data << ':' << m_line << '\n';
}

Logger::Logger(SourceFile file, int line) :
    m_impl(LogLevel::INFO, 0, file, line) {

}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : m_impl(level, 0, file, line)
{
    m_impl.m_stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : m_impl(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : m_impl(toAbort ? FATAL : ERROR, errno, file, line)
{
}


Logger::~Logger()
{
    m_impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (m_impl.m_level == FATAL)
    {
        g_flush();
        // abort();
    }
}


void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}

// void Logger::setTimeZone(const TimeZone& tz)
// {
//     g_logTimeZone = tz;
// }
