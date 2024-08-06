/*
    测试内容
    1. 本线程调用runAfter, runAt, runEvery这三函数
    2. 其他线程调用runAfter, runAt, runEvery这三函数
*/


#include <eventLoop.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


using namespace std;

EventLoop* g_loop;

void print() {
    printf("print(): Pid = %d, tid = %lu\n", getpid(), pthread_self());
}

void* threadFunc(void* arg) {
    g_loop->runAfter(3, print);
    g_loop->runAt(Timestamp::now() + 3, print);
    g_loop->runEvery(3, print);
    g_loop->loop();
    return nullptr;
}



int main() {
    EventLoop loop("epoll");
    g_loop = &loop;
    /*
        测试本线程调用runAfter, runAt, runEvery这三函数`
    */
    // g_loop->runAfter(3, print);
    // g_loop->runAt(Timestamp::now() + 3 * 1000000, print);
    g_loop->runEvery(3, print);
    g_loop->loop();

    /*
        测试其他线程调用runAfter, runAt, runEvery这三函数
    */
    // pthread_t tid;
    // pthread_create(&tid, nullptr, threadFunc, nullptr);
    // pthread_join(tid, nullptr);

    g_loop->quit();
    return 0;
}
