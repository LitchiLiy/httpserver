// 创建一个TCP客户端
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <unistd.h>




using namespace std;

int main() {
    const char ip[] = "127.0.0.1";
    short port = 8888;
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    cout << "ret = " << ret << endl;
    char sentBuf[1024];
    int ppp = 1;
    // 发送数据给对方
    while (ppp) {
        cout << "请输入要发送的数据：" << endl;
        cin.getline(sentBuf, 1024);
        if (!strcmp(sentBuf, "exit")) {
            break;
        }
        int len = strlen(sentBuf);
        int rr = write(sockfd, sentBuf, len);
        cout << "rr = " << rr << endl;
    }

    shutdown(sockfd, SHUT_WR);

    return 0;
}