#include <sys/timerfd.h>

#include <eventLoop.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <channel.h>
#include <epoller.h>

int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
EventLoop* g_loop;
void timeout(void) {
    char rr[1024];
    read(timerfd, rr, 1024);  // 一定要把它读出来, 不然LT模式一直触发
    std::cout << "timeout" << std::endl;
    // g_loop->quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    Channel ch(&loop, timerfd);
    ch.setReadCallBack(timeout);
    ch.setReadEnable();

    struct itimerspec howlong;
    // bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();
    close(timerfd);
    return 0;
}

