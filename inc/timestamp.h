#if !defined(TIMERSTAMP_H)
#define TIMERSTAMP_H

#include <sys/timerfd.h>
#include <stdint.h>
#include <algorithm>
#include <bits/types.h>
#include <string>
using namespace std;

/*
    用法, 输入一个usec_t类型的时间戳, 而且这个时间戳是相对与纪元前的, 故可能还要调用now函数来获得相对于纪元前的时间
*/



class Timestamp {
public:
    typedef int64_t usec_t;
    Timestamp();
    ~Timestamp();
    Timestamp(usec_t usec); // 单个参数传入微秒
    Timestamp(time_t sec, usec_t usec);

    time_t secondsSinceEpoch() const { return m_usec / 1000000; }

    usec_t showusec() const { return m_usec; }
    time_t showsec() const { return static_cast<time_t>(m_usec / 1000000); }
    bool isVaild() const { return m_usec > 0; }
    void timestampSwap(Timestamp& that) { std::swap(m_usec, that.m_usec); }


    string toFormatString() const {
        time_t secNow = showsec();
        struct tm tm_;
        gmtime_r(&secNow, &tm_);
        char buf[64];
        snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d",
                 tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
                 tm_.tm_hour, tm_.tm_min, tm_.tm_sec);
        return buf;
    }

    // 提供一些函数操作
    static Timestamp now();  // 获取当前时间
    static Timestamp fromUnixTime(time_t t)  // 时间转换
    {
        return fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * 1000000 + microseconds);
    }

    Timestamp operator+(const Timestamp& rhs) const {
        return Timestamp(m_usec + rhs.showusec());
    }


private:
    usec_t m_usec; // 只记录来自纪元的微秒数.
};




inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.showusec() < rhs.showusec();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.showusec() == rhs.showusec();
}

// 计算秒的时间差
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.showusec() - low.showusec();
    return static_cast<double>(diff) / 1000000;
}


#endif // TIMERSTAMP_H
