#include <tcpClient.h>
#include <mInetAddress.h>
#include <eventLoop.h>
#include <buffer.h>
#include <timestamp.h>
#include <tcpConnection.h>
#include <connector.h>

#include <string>
#include <iostream>


void defaultConnectionCb(const TcpConnectionPtr& conn) {
    std::cout << "Connection success" << endl;

}

void defaultMessageCb(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    buf->retrieveAll();
}


using namespace std;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& name) :
    m_loop(loop),
    m_connector(new Connector(loop, serverAddr)),
    m_name(name),
    m_connectionCallback(defaultConnectionCb),
    m_messageCallback(defaultMessageCb),
    isretryEnable(false),
    isconnected(true),
    m_nextConnId(1)
{
    m_connector->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
void removeConnector(const ConnectorPtr& connector)
{
    //connector->
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique;
    {
        lock_guard<mutex> lock(m_mutex);
        unique = m_TcpConnection.unique();
        conn = m_TcpConnection;
    }
    if (conn) {
        assert(m_loop == conn->getLoop());
        CloseCallback cb = std::bind(&::removeConnection, m_loop, std::placeholders::_1);  // 这里调用的是外部的removeConntion
        m_loop->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    }
    else {
        m_connector->stopCntor();
        m_loop->runAfter(1, std::bind(&removeConnector, m_connector)); // 调用外部的
    }
}


void TcpClient::connect() {
    isconnected = true;
    m_connector->restartCntor();
}

void TcpClient::disconnect() {
    isconnected = false;
    {
        lock_guard<mutex> lock(m_mutex);
        if (m_TcpConnection) {
            m_TcpConnection->shutdown();
        }
    }
}

void TcpClient::stop() {
    isconnected = false;
    m_connector->stopCntor();
}

void TcpClient::newConnection(int sockfd)
{
    m_loop->assertInLoopThread();
    InetAddress peerAddr(InetAddress::getPeerAddr(sockfd));
    ++m_nextConnId;
    string connName = to_string(m_nextConnId);
    InetAddress localAddr(InetAddress::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(m_loop,
        connName,
        sockfd,
        localAddr,
        peerAddr));

    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
    {
        lock_guard<mutex> lock(m_mutex);
        m_TcpConnection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->assertInLoopThread();
    assert(m_loop == conn->getLoop());
    {
        lock_guard<mutex> lock(m_mutex);
        assert(m_TcpConnection == conn);
        m_TcpConnection.reset();
    }

    m_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (isretryEnable && isconnected)
    {
        m_connector->restartCntor();
    }
}
