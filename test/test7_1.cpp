#include <eventLoop.h> 
#include <iostream>
#include <mAcceptor.h>
#include <mInetAddress.h>
#include <unistd.h>
/*
1. 先打开服务器, 然后再打开客户端程序, 客户端程序收到hello, 服务器发出connect
*/


using namespace std;

void newConnection(int sockfd, const InetAddress& peerAddr) {
    // 拿到客户端的fd之后, 往里面写内容
    cout << "newConnection" << endl;
    char senBuf[] = { "Hello, what is your name?" };  // 
    send(sockfd, senBuf, sizeof(senBuf), 0);
    // write(sockfd, "How are you?\n", 13);
    close(sockfd); // 我们这边主动关闭
}

int main() {
    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 8888, false);
    Acceptor acceptor(&loop, listenAddr, true);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
    return 0;
}