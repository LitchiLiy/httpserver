#if !defined(TIMER_H)
#define TIMER_H
/*
    只是用来装信息的, 里边不包含任何定时器的fd
*/



#include <functional>
#include <stdint.h>
#include <timestamp.h>
#include <atomicInt.h>




typedef std::function<void()> callBack_f;
typedef int64_t seq_t;
class Timer {
public:
    Timer(callBack_f cb_, Timestamp when_, double interval_);
    ~Timer();

public:

    void runTimerCallBack();
    Timestamp showExpired() const { return m_when; }
    bool isRepeat() const { return is_Repeat; }
    seq_t showSeq() const { return m_seq; }
    void setSeq(seq_t seq) { m_seq = seq; }

    void restart(Timestamp now);


private:
    callBack_f m_cb;
    Timestamp m_when;
    double m_interval;  // 单位为秒
    bool is_Repeat;

    seq_t m_seq; // 用到序列号的理由, Timer类没有序列, 定时器又只有一个fd号, 必须自己构件一个原子唯一的序列号

    static AtomicInt64 m_AutomaticInt64; // 全局静态, 所有类实例共有的一个变量, 用来控制序列号

};

#endif // TIMER_H
