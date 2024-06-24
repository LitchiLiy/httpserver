#if !defined(TIMERID_H)
#define TIMERID_H


#include <memory>
#include <algorithm>
#include <stdint.h>


class Timer;
typedef int64_t seq_t;

class TimerId {
private:
    Timer* m_timer;
    seq_t m_seq;

public:
    TimerId(Timer* timer, seq_t seq) : m_timer(std::move(timer)), m_seq(seq) {

    }

    friend class TimerQueue; // 作为友元, 其内部函数是可以调用TimerQueue的私有变量的
};

#endif // TIMERID_H
