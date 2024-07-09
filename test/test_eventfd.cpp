#include <sys/eventfd.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <sys/epoll.h>
#include <assert.h>

using namespace std;
int efd = eventfd(0, EFD_CLOEXEC);
std::mutex mtx1;
void* create(void* arg) {
    {
        lock_guard<mutex> Log(mtx1);
        cout << "准备开始发送epoll咯" << endl;
    }
    sleep(3); // 10s
    eventfd_t ui64 = 10;
    eventfd_write(efd, ui64);
    {
        lock_guard<mutex> Log(mtx1);
        cout << "我写完了" << endl;
    }
    return nullptr;
}
int main() {
    int epollfd = epoll_create1(0);
    epoll_event event1, event2[2];
    event1.data.fd = efd;
    event1.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, efd, &event1);
    pthread_t pPtr;
    int pfd = pthread_create(&pPtr, nullptr, create, nullptr);
    pthread_detach(pPtr);
    while (true)
    {
        int ret = epoll_wait(epollfd, event2, 2, -1);
        if (ret > 0) {
            for (int i = 0; i < ret; i++) {
                int ffd = event2[i].data.fd;
                assert(ffd == efd);
                eventfd_t reade;
                // eventfd_read(ffd, &reade);  // 如果不读,则一直触发.LT
                mtx1.lock();
                cout << reade << endl;
                mtx1.unlock();
            }
        }
    }

    return 0;
}


