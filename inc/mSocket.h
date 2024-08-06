#if !defined(MSOCKET_H)
#define MSOCKET_H


#include <sys/socket.h>


/*
SOCKET类的功能
1. bind, listen, accept
2. 设置这个socket的各种功能, 比如reuseaddr, keepalive, reuseport
3. 保存着一个socketfd实例. 但是内部不创造实例, fd由外部提供]


给你一个socket的fd, 让这个类包管绑定, 监听, 接收这套流程包括设置各种功能
*/


class InetAddress;
class Socket {
public:
    Socket(int sockfd);
    Socket() = default;
    ~Socket();

    int fd() const { return m_socketFd; }
    void setFd(int sockfd) { m_socketFd = sockfd; }

    void bindAddress(const InetAddress& localaddr);
    void startListen(int num);
    int accept(InetAddress* peeraddr);

    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    void shutdownSockfd();
    void setTcpNoDelay(bool on);

    void shutdownWrite() { shutdown(m_socketFd, SHUT_WR); }; // 禁用该fd的发送操作.

private:
    int m_socketFd;
};



#endif // MSOCKET_H)
