#if !defined(TIMERSTAMP_H)
#define TIMERSTAMP_H

#include <sys/timerfd.h>
#include <stdint.h>
#include <algorithm>

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

    // 提供一些函数操作
    static Timestamp now();  // 获取当前时间
    static Timestamp fromUnixTime(time_t t)  // 时间转换
    {
        return fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(time_t t, int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * 1000000 + microseconds);
    }

    Timestamp operator+(const Timestamp& rhs) const {
        return Timestamp(m_usec + rhs.showusec());
    }


private:
    usec_t m_usec; // 只记录来自纪元的微秒数.
};




inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.showusec() < rhs.showusec();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.showusec() == rhs.showusec();
}

#endif // TIMERSTAMP_H
