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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    cout << "ret = " << ret << endl;
    char recvBuf[1024];
    int ppp = 1;
    while (ppp)
    {
        memset(recvBuf, 0, sizeof(recvBuf));
        int rett = recv(sockfd, recvBuf, sizeof(recvBuf), 0); // 一直阻塞
        if (rett > ppp) {
            ppp = 0;
        }
        cout << recvBuf << endl;
    }
    close(sockfd);
}