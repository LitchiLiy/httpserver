#if !defined(TCPCLIENT_H)    
#define TCPCLIENT_H

/*
功能
1. 用Connector不断地保持连接地建立和发起连接
2. 用TcpConnector在连接过程中接收和发送信息。
*/



#include <string>
#include <callBacks.h>
#include <mutex>
#include <connector.h>




class InetAddress;
class EventLoop;

using namespace std;

class TcpClient {
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& name);
    ~TcpClient();


    void connect();
    void disconnect();
    void stop();


    TcpConnectionPtr connection() {  // 这里不能声明为const, 因为你要改变类内的mutex的状态
        lock_guard<mutex> lock(m_mutex);
        return m_TcpConnection;
    }

    EventLoop* getLoop() { return m_loop; }
    bool isRetryEnable() const { return isretryEnable; }
    void enableRetry() { isretryEnable = true; }
    const string name() const { return m_name; }

    void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = move(cb); }
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = move(cb);; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = move(cb);; }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);



private:
    EventLoop* m_loop;
    ConnectorPtr m_connector;
    const string m_name;
    mutex m_mutex;
    TcpConnectionPtr m_TcpConnection;

    bool isretryEnable;
    bool isconnected;

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;

    int m_nextConnId;

};


#endif // TCPCLIENT_H)   
