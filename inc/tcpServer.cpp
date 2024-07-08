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
#include <logging.h>




using namespace std;


void defaultConnectionCallback(const TcpConnectionPtr& conn) {
    // LOG_TRACE << conn->localAddress().toIpPort() << " -> "
    //     << conn->peerAddress().toIpPort() << " is "
    //     << (conn->connected() ? "UP" : "DOWN");
    LOG_INFO << "TcpConnect is " << (conn->isConnected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register message callback only.
}

void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp);


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg, TcpServer::Option option) :
    m_loop(loop),
    m_ipPort(listenAddr.ipToString()),
    m_name(nameArg),
    m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
    m_threadPool(new EventLoopThreadPool(loop, nameArg)),
    m_connectionCallback(defaultConnectionCallback), // 这里确实将默认函数放进去了，已经测试过了
    m_messageCallback(defaultMessageCallback),
    nextConnId(1) {
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)); // 占位符, 表示调用时要传入两个参数
    LOG_INFO << "TcpServer listen on " << listenAddr.ipToString();

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
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
    // 如果原子操作的值是0
    if (m_started.incrementAndGet() == 1) { // 初始化为0, 将这个值加1之后返回加完之后的值.
        // 初始化线程池
        m_threadPool->startPool(m_threadInitCallback);  // 这里才开始构建ELT实例, 这个初始化函数在, 就执行, 不在, 就不执行在Thread中有这个逻辑
        assert(!m_acceptor->islistening());
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
    // // 如果你还没有搞好线程池, 就先暂时用这个, 搞好了再把这个注释掉
    // m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
}

sockaddr_in TcpServer::getSockAddr(int sockfd) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) < 0) { // 获取客户端sockfd连接的那个服务器地址
        perror("getsockname");
    }
    return addr;
}



void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    m_loop->assertInLoopThread();
    EventLoop* threadLoop = m_threadPool->getNextLoop(); // tcpserver只有一个acceptor, 然后调用线程池中的一个线程来对接这个客户端
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", m_ipPort.c_str(), nextConnId);



    string connName = m_name + buf; // 客户端的名字由： 服务器名字 + 服务器的ip + 客户端在服务器内部的编号组成
    ++nextConnId;

    // 这里从外部的sockfd获取他的本地地址
    InetAddress localAddr(TcpServer::getSockAddr(sockfd)); // 服务器可能不止用一个fd来开放listen, 可能有多个本地listenfd, 这里获取客户端连接的那个
    // 创建一个connection, 这里其实传入的是线程池的loop, 但是为了测试我们这里放我们自己的
    // 实际测试: local和peer两个是不一样的
    TcpConnectionPtr conn(new TcpConnection(threadLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    LOG_INFO << "TcpServer::newConnection [" << this->name() << "] - new connection [" << conn->name() << "] from " << peerAddr.ipToString();


    // LOG_INFO << conn.use_count();

    m_connMap[conn->name()] = conn;
    // LOG_INFO << conn.use_count();
    // 这里设置回调函数
    // m_connectionCallback(conn);
    conn->setConnectionCallback(m_connectionCallback);   // 外部构建tcpserver是set的mconnectCb
    // m_connectionCallback(conn);
    conn->setMessageCallback(m_messageCallback);    // 外部构建的想让服务器怎么回应的cb
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 在主线程里调用其他线程的EL的runInLoop
    threadLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn)); // 暂时用tcpServer的loop代替
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    m_loop->assertInLoopThread();
    size_t n = m_connMap.erase(conn->name());
    // 线程池操作
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
    // m_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp) {
    buf->retrieveAll();
}