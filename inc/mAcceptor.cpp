#include <mAcceptor.h>
#include <eventLoop.h>
#include <mInetAddress.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mSocket.h>


#include <channel.h>



Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr) {
    // 首先创建一个socket, 设置为非阻塞
    m_acceptFd = socket(listenAddr.getSockAddr().sin_family, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(m_acceptFd, SOL_SOCKET, SO_REUSEADDR | SOCK_NONBLOCK | SOCK_CLOEXEC, &opt, sizeof(opt));
    if (m_acceptFd < 0) {
        perror("socket");
    }
    // 设置m_socket
    m_socket.setFd(m_acceptFd);
    m_socket.setReuseAddr(true);
    m_socket.setReusePort(true);
    m_socket.bindAddress(listenAddr); // 初始化只做到绑定就行了
    m_loop = loop;
    // 初始化channel
    m_channel = Channel(loop, m_acceptFd);
    m_channel.setReadCallBack(std::bind(&Acceptor::handleRead, this));
    // 并不使能读. 等开始listen的时候才使能.
}

Acceptor::~Acceptor()
{
    m_channel.disableAll();
    m_channel.remove();
    close(m_acceptFd);
}


void Acceptor::listen() { // 简单listen, 然后使能channel, 当你要开始监听的时候才开始使能.
    m_loop->isInLoopThread();
    m_socket.startListen(10);
    m_channel.setReadEnable();
    isListening = true;
}

void Acceptor::handleRead() {
    // 对方申请连接,  接收fd, 然后调用回调函数
    m_loop->isInLoopThread();
    InetAddress peerAddr;
    int connfd = m_socket.accept(&peerAddr);
    if (connfd >= 0) {
        if (m_newConnectionCallback) {
            m_newConnectionCallback(connfd, peerAddr); // 执行保存在Acceptor的新连接回调, 这个回调会执行TcpServer的新连接函数
        }
        else {
            close(connfd);
        }
    }
    else {
        perror("accept");
    }
}
