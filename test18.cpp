/*
    测试ET模式怎么弄
*/

#include <sys/epoll.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
int main() {
    int ret1 = 111;
    int serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    int on = 1;
    ret1 = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    ret1 = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
        printf("bind error\n");
    }
    if (listen(serverfd, 10) == -1) {
        printf("listen error\n");
    }
    struct epoll_event ev1, ev2[10];
    ev1.data.fd = serverfd;
    ev1.events = EPOLLIN | EPOLLET; // epoll ET 模式， 还有非阻塞
    int epfd = epoll_create1(0); // 注册事件表
    if (epfd == -1) {
        printf("epoll create error\n");
    }
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, serverfd, &ev1);
    int tt = 1;
    while (tt) {
        printf("wait\n");
        int n = epoll_wait(epfd, ev2, 10, -1);
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                if (ev2[i].data.fd == serverfd) {
                    int clientfd = accept4(serverfd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
                    ev2[i].data.fd = clientfd;
                    ev2[i].events = EPOLLIN | EPOLLET; // epoll ET 模式， 还有非阻塞
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev2[i]); // 添加事件
                }
                else { // 收到客户端的send数据， ET模式读取数据
                    char buf[10];
                    while (true) {
                        int len = recv(ev2[i].data.fd, buf, 10, 0); // 0 为默认行为

                        if (len > 0) {
                            if (len != 10) {  // recv不会加\0， 需要手动加
                                buf[len] = '\0';
                            }
                            // 此时有数据读出， 继续读
                            printf("%s", buf);
                        }
                        else if (len == -1) {
                            // 如果是没有数据可读， 则会EAGAIN和EWOULDBLOCK
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                // 此时没有数据
                                printf("\n ET over read\n");
                                break; // 退出即可
                            }
                            else {
                                // 此时是其他错误， 直接退出
                                printf("\n ERROR\n");
                                break;
                            }
                        }
                        else if (len == 0) {
                            // 此时是对方关闭了， 直接退出
                            printf("\n client close\n");
                            close(ev2[i].data.fd);
                            tt = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
