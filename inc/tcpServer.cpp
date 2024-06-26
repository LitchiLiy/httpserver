#include <tcpServer.h>
#include <eventLoop.h>
#include <mInetAddress.h>
#include <mAcceptor.h>
#include <timestamp.h>
#include <buffer.h>
#include <tcpConnection.h>
#include <sys/socket.h>
#include <string.h>
#include <callBacks.h>



using namespace std;

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    std::cout << "new connection: " << std::endl;
}

void defaultMessageCallback(const TcpConnectionPtr& conn,
    Buffer* buf,
    Timestamp);


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg) :
    m_loop(loop),
    // m_ipPort(listenAddr.ptonIpPort()), 这个很复杂, 先放着
    m_name(nameArg),
    m_acceptor(new Acceptor(loop, listenAddr)),
    // m_threadPool(new EventLoopThreadPool(loop, nameArg)),
    m_connectionCallback(defaultConnectionCallback),
    m_messageCallback(defaultMessageCallback),
    nextConnId(1)
{
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)); // 占位符, 表示调用时要传入两个参数
}

// 循环对我们保存的connectionMap进行destroy.
TcpServer::~TcpServer() {
    m_loop->assertInLoopThread();
    for (auto& item : m_connMap) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    // m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
    // 如果原子操作的值是0
    if (m_started.incrementAndGet() == 0) {
        // 初始化线程池
        // m_threadPool->start(threadInitCallback);
        assert(!m_acceptor->islistening());
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}

sockaddr_in TcpServer::getSockAddr(int sockfd) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) < 0) { // getsockname获取sockfd的本地地址, 或者理解为获取套接字绑定的端口和地址
        perror("getsockname");
    }
    return addr;
}



void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    m_loop->assertInLoopThread();
    // EventLoop* threadLoop = m_threadPool->getNextLoop();
    string connName = to_string(nextConnId);
    ++nextConnId;

    // 这里从外部的sockfd获取他的本地地址
    InetAddress localAddr(TcpServer::getSockAddr(sockfd));
    // 创建一个connection, 这里其实传入的是线程池的loop, 但是为了测试我们这里放我们自己的
    TcpConnectionPtr conn(new TcpConnection(m_loop,
        connName,
        sockfd,
        localAddr,
        peerAddr));

    m_connMap[conn->name()] = conn;
    // 这里设置回调函数
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 线程池调用connnectEstabilsehd
    // m_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    m_loop->assertInLoopThread();
    size_t n = m_connMap.erase(conn->name());
    // 线程池操作
    // EventLoop* ioLoop = conn->getLoop();
    // ioLoop->queueInLoop(
    //     std::bind(&TcpConnection::connectDestroyed, conn));
    m_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void defaultMessageCallback(const TcpConnectionPtr& conn,
    Buffer* buf,
    Timestamp) {
    buf->retrieveAll();
}