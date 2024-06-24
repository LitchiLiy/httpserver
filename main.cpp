#include <eventLoop.h>
#include <unistd.h>

EventLoop* ptr;

void* threadFunc(void* arg) {
    printf("thredFunc(): Pid = %d, tid = %lu\n", getpid(), pthread_self());
    ptr->loop();
    return nullptr;
}

int main() {
    printf("mian(): Pid = %d, tid = %lu\n", getpid(), pthread_self());

    EventLoop loop1;
    ptr = &loop1;
    pthread_t ptd;
    pthread_create(&ptd, nullptr, &threadFunc, nullptr);
    pthread_exit(nullptr);
    return 0;
}


