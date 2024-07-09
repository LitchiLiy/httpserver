/*
    测试eventLoopThread.h的功能
*/


#include <stdio.h>
#include <unistd.h>
#include <eventLoopThread.h>
#include <eventLoop.h>
#include <thread.h>
#include <functional>


using namespace std;

void print(EventLoop* p = NULL)
{
    printf("print: pid = %d, tid = %ld, loop = %p\n",
        getpid(), pthread_self(), p);
}

void quit(EventLoop* p)
{
    print(p);
    p->quit();
}

int main()
{
    print();
    {
        EventLoopThread thr1;  // never start
    }
    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
        while (1) {}
    }
    // {
    //     // quit() before dtor
    //     EventLoopThread thr3;
    //     EventLoop* loop = thr3.startLoop();
    //     loop->runInLoop(std::bind(quit, loop));
    // }
}

