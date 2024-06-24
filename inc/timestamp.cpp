#include <timestamp.h>
#include <sys/time.h>

Timestamp::Timestamp() {
    m_usec = 0;
}


Timestamp::Timestamp(time_t sec, usec_t usec) {
    m_usec = static_cast<usec_t>(sec) * 1000000 + usec;

}

Timestamp::~Timestamp() {
}

Timestamp::Timestamp(usec_t usec) {
    m_usec = usec;
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(0, seconds * 1000000 + tv.tv_usec);
}

