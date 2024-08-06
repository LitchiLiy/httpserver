#if !defined(TCPSERVER_H)
#define TCPSERVER_H

/*
作用: 调用acceptor来接收新的连接, 掌管着一个EventLoopThread的线程池, 还由创建新连接的函数api

建立的新连接都放到线程池里面去了. TcpServer只是管理这个线程池.
*/

#include <string>
#include <callBacks.h>
#include <memory>
#include <atomicInt.h>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <eventLoopThreadPool.h>


// 


class EventLoop;
class InetAddress;
class Acceptor;
class TcpConnection;
class Buffer;
class Timestamp;
class EventLoopThreadPool;

using namespace std;

class TcpServer {
    typedef std::map<string, TcpConnectionPtr> ConnectionMap;
public:
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop* el,
              const InetAddress& listenAddr,
              const std::string& name,
              Option option = kNoReusePort);
    ~TcpServer();



    // 展示自己
    const string& ipPort() const { return m_ipPort; }
    const string& name() const { return m_name; }
    EventLoop* getLoop() const { return m_loop; }
    // shared_ptr<EventLoopTHreadPool> showThreadPool() { return m_threadPool; }

    void start();


    void setThreadNum(int numThreads);
    void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
    void setThreadInitCallback(const threadInitCallBack& cb) { m_threadInitCallback = cb; }



private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    sockaddr_in getSockAddr(int sockfd);






private:

    EventLoop* m_loop;
    std::string m_ipPort;
    std::string m_name;
    std::shared_ptr<Acceptor> m_acceptor;
    std::shared_ptr<EventLoopThreadPool> m_threadPool;

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    threadInitCallBack m_threadInitCallback;
    AtomicInt32 m_started;

    int nextConnId;

    ConnectionMap m_connMap; // TcpServer保存着所有的连接ptr, 所有与客户端的连接的唯一保存地


};



#endif // TCPSERVER_H)
