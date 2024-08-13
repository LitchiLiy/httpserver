#include <logFile.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <time.h>
#include <mutex>
#include <pthread.h>




// 打开文件, 设置偏移量， 输入的filename是相对与bin的Main的路径
AppendFile::AppendFile(const string& filename) : fp_(fopen(filename.c_str(), "ae")) {
    writtenBytes_ = 0;
    assert(fp_ != nullptr); // 这里出错， 说明文件夹不存在， 你先创建一个文件夹
    ::setbuf(fp_, buffer_); // 自定义一个缓冲区, 提高效率
}

AppendFile::~AppendFile() {
    ::fclose(fp_);
}


// 往文件中写入logline信息
void AppendFile::append(const char* logline, const size_t len) {
    size_t written = 0;

    while (written != len) {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        // if (n != remain)
        // {
        //     int err = ferror(fp_);
        //     if (err)
        //     {
        //         fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
        //     }
        //     break;
        // }
        written += n;
    }
    writtenBytes_ += written;
}


void AppendFile::flush() {
    ::fflush(fp_);
}

// 往文件中写入logline信息
size_t AppendFile::write(const char* logline, size_t len) {
    return ::fwrite_unlocked(logline, 1, len, fp_);  // unistd.h中的
}




LogFile::LogFile(const string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
    : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    // mutex_(threadSafe ? new MutexLock : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0) {
    // assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
    {
        lock_guard<mutex> lock(m_mutex);
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    {
        lock_guard<mutex> lock(m_mutex);
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len); // 单纯的往文件中写入logline信息

    if (file_->writtenBytes() > rollSize_) // 我们设定了一个字节滚动阈值, 如果写入的字节数大于这个阈值, 那么就换个文件夹
    {
        rollFile();
    }
    else {
        ++count_;  // 日志追加计数器
        if (count_ >= checkEveryN_) // 检查计数阈值, 超过了就执行., 这段的意思就是, 如果不能换个文件夹, 那么至少要把文件中的数据flush进去, 及时写入文件中避免丢失.
        {
            count_ = 0;
            time_t now = ::time(NULL); // 公元前至今的时间戳
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)  // 当前事件与上一次事件的差值超出了阈值.
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

// 日志滚动, 就是换文件的意思. 这个文件大小差不多了, 就会换文件. 只负责换文件的义务
bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);  // 拼接文件名, 带路径的
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_; // 出基于当前时间 now 的日志文件的起始时间

    // 这里纯粹是为了防止瞬时触发
    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));  // 新建文件, 构造函数打开了一个fd为filename的文件
        return true;
    }
    return false;
}


// 这里设定文件名, Logfile_20240701-221705_pid=121137.log
string LogFile::getLogFileName(const string& basename, time_t* now) {
    string filename;
    filename.reserve(basename.size() + 64);  // 预留足够的空间
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm); // localtime_r获取的是中国时间
    strftime(timebuf, sizeof(timebuf), "_%Y%m%d-%H%M%S_", &tm);
    filename += timebuf;

    // char buf[256];
    // gethostname(buf, sizeof buf);
    // filename += string(buf, sizeof buf - 1);


    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, "pid=%d", getpid());
    filename += pidbuf;

    filename += ".log";

    return filename;
}

