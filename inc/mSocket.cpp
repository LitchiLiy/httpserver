#include <mSocket.h>
#include <mInetAddress.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <logging.h>
#include <errno.h>



Socket::Socket(int sockfd) : m_socketFd(sockfd) {}

Socket::~Socket() {

}


void Socket::bindAddress(const InetAddress& localaddr) {
    struct sockaddr_in addr = localaddr.getSockAddr();
    int ret = bind(m_socketFd, (sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR << "bind error: " << strerror(errno);
    }
}


void Socket::startListen(int num = 10) {
    int ret = listen(m_socketFd, num); // 默认是10
    // 开始listen
    if (ret < 0) {
        LOG_ERROR << "listen error: " << strerror(errno);
    }
}

/*
    @brief 输入一个地址类, 通过服务器socket, 返回这个地址信息给你, 并且返回一个客户端fd
*/
int Socket::accept(InetAddress* peeraddr) {

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int connfd = accept4(m_socketFd, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC); // accept普通的accept多一个flag符号, 这里默认连接是非阻塞的, 这表明到时候根据这个fd读数据的时候, 不会阻塞.
    int on = 1;
    setsockopt(connfd, SOL_SOCKET, TCP_NODELAY, &on, sizeof on);
    // 输出收到的fd的任何信息
    {
        char ipstr[50];
        inet_ntop(AF_INET, &(addr.sin_addr), ipstr, sizeof ipstr);
        // std::cout << "Accepted new connection of fd: " << connfd << " Client address: " << ipstr << ":" << ntohs(addr.sin_port) << std::endl;
    }
    if (connfd < 0) {
        LOG_ERROR << "accept error: " << strerror(errno);
    }
    peeraddr->setSockAddr(addr);  // 保存地址
    return connfd;
}


void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

// 设置reuseport, 这东西有可能不支持.
void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
}

// 设置keepalive, 它的作用是如果在idle时间内没有数据交互, 就发送一个keepalive probes
void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(m_socketFd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}


void Socket::shutdownSockfd() {
    shutdown(m_socketFd, SHUT_WR);
}

// 改变 TCP 协议的 Nagle 算法 的行为。Nagle 算法是一种用于改善网络通信效率的算法，由 John Nagle 提出。其核心思想是将小的分组合并为一个较大的分组，减少网络中分组的数量，从而降低因大量小分组而产生的网络拥塞。
void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}