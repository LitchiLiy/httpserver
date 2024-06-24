#include <timer.h>

AtomicInt64 Timer::m_AutomaticInt64; // 定义静态类的声明, 这里没写一直出错

Timer::Timer(callBack_f cb_, Timestamp when_, double interval_) :
    m_cb(cb_),
    m_when(when_),
    m_interval(interval_),
    is_Repeat(interval_ > 0.0)
{
    m_seq = m_AutomaticInt64.incrementAndGet();
}


Timer::~Timer() {
}

void Timer::runTimerCallBack() {
    m_cb();
}


void Timer::restart(Timestamp now) {
    if (is_Repeat) {
        m_when = now.showusec() + static_cast<Timestamp::usec_t>(m_interval * 1000000);
    }
    else {
        m_when = Timestamp();
    }
}
