#if !defined(LOGGING_H)
#define LOGGING_H

/*
作用: 将前端的buffer转移到后端, 然后将后端的buffer输入到文件中
*/

#include <timestamp.h>
#include <logStream.h>


/*
Logger:最终为了实现功能, 将日志输出到文件中, 调用Logger中的LogStream对象, 将日志内容输出到文件中
*/



class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };


    // 这个类是用来记录日志的文件名的, 构建类的时候直接得到文件名.
    class SourceFile {
    public:
        template<int N>
        SourceFile(const char(&arr)[N]) :m_data(arr), m_size(N - 1) {
            const char* slash = strchr(m_data, '/');  // 找最后一个出现的地址
            if (slash) {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
        }

        explicit SourceFile(const char* filename) : m_data(filename) {
            const char* slash = strchr(m_data, '/');
            if (slash) {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
        }

        const char* data() const { return m_data; };
        int size() const { return m_size; };

        const char* m_data;
        int m_size;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return m_impl.m_stream; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc out);
    static void setFlush(FlushFunc flush);
    // static void setTimeZone(const char* zone);



    class Impl {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp m_time;
        LogStream m_stream;
        LogLevel m_level;
        int m_line;
        SourceFile m_file;
    };

private:
    Impl m_impl;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

// 先判断日志级别对不对, 如果对, 则将LOG_TRACE定义为一个Logger.stream的对象, 到时候直接调用LOG_TRACE<<"hello"就行了 
// 文件名和line是放在数据的后面的
#define LOG_TRACE if(Logger::logLevel() <= Logger::TRACE) Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream() 
#define LOG_DEBUG if(Logger::logLevel() <= Logger::DEBUG) Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream() 
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()










#endif // LOGGING_H)

