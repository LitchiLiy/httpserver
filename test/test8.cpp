#include <iostream>
#include <timestamp.h>
#include <eventLoop.h>

/*
主要是测试channel的receive增加了timestamp之后程序还能不能跑通, 大概的意思就是, 每次epoll收到客户端发来的信息之后会记录收到信息时的时刻, 然后再处理函数的地方你可以再now一次, 来看看从收到信息到处理事件之间的延迟时多少, 从这里开看时10微秒.
*/


EventLoop* g_loop;




void timeout(Timestamp recTime) {
    std::cout << "timeout before the timestamp is: " << recTime.showsec() << std::endl;

    std::cout << "timeout the now timestamp is: " << Timestamp::now().showsec() << std::endl;

    g_loop->quit();
}

int main() {
    // std::cout << "start timestamp is: " << Timestamp::now().showsec() << std::endl;
    EventLoop loop;
    g_loop = &loop;
    loop.runAfter(3, std::bind(&timeout, Timestamp::now()));
    loop.loop();
}

