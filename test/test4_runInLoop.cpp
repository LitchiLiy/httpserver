#include <eventLoop.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/*
    1. 测试runInLoop, 主要功能就是本线程调用就是直接执行回调函数, 其他线程调用就是调用queueInLoop
    2. 不是本线程调用queueInLoop, 它会将cb加入一个数组中, 如果不是本线程就wakeup
    3. 那个额外的数组会在本线程的epoll最后执行.
    也就是至始至终都是在本线程执行runInLoop的cb.

    const function<void()> 类型的形参, 可以直接将run丢进去. 而不用用run构建一个callBack_t

    1. 实现了runInLoop, 和wakeup的功能
*/

EventLoop* g_loop;
int num = 0;
using namespace std;
void run() {
    printf("run(): Pid = %d, tid = %lu\n", getpid(), pthread_self());
}

void run1() {
    // 本线程调用
    printf("run1(): Pid = %d, tid = %lu\n", getpid(), pthread_self());
    g_loop->runInLoop(run);
}

void* run2(void* arg) {
    EventLoop* loop = (EventLoop*)arg;
    // 其他线程调用
    printf("run2(): Pid = %d, tid = %lu\n", getpid(), pthread_self());
    loop->runInLoop(run1);
    return nullptr;
}
int main() {
    EventLoop loop;
    g_loop = &loop;
    // 本线程调用
    g_loop->runInLoop(run1);  // 在本线程, 故直接输出, 
    // 其他线程调用
    pthread_t tid;
    pthread_create(&tid, nullptr, run2, g_loop);  // 只要我在loop那里打断点, 这个线程调用的内容不会输出, 因为在queueInLoop的队列里, wake虽然调用了, 但是我不epoll就识别不到wakeup
    pthread_detach(tid);
    g_loop->loop();  // 这里尝试断点
    g_loop->quit();
    return 0;
}

