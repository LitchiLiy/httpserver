#if !defined(TCPCONNECTION_H)
#define TCPCONNECTION_H

#include <memory>
#include <string>
#include <stringPiece.h>

#include <mInetAddress.h>
#include <callBacks.h>
#include <buffer.h>
#include <nocopyable.h>

#include <httpContext.h>

/*
输入一个loop， 一个name， 一个fd, 一个localAddr, 一个peerAddr
读数据， 写数据， 关闭连接， 有错误, 保存着对应行动的cb,
提供读数据, 写数据调用的api,
记录着信息, 没有连接操作, 而是收到连接成功之后再创建tcpconnection.
*/



class EventLoop;
class Socket;
class Channel;
class stringPiece;


class TcpConnection :
    public std::enable_shared_from_this<TcpConnection> {
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
    TcpConnection() = default;
    ~TcpConnection();

    // 供外部调用, 展示类内信息
    EventLoop* getLoop() const { return m_loop; }
    const std::string& name() const { return m_name; }
    const InetAddress& localAddress() const { return m_localAddr; }
    const InetAddress& peerAddress() const { return m_peerAddr; }
    bool isConnected() const { return m_state == kConnected; }
    bool isDisconnected() const { return m_state == kDisconnected; }


    // 写相关
    void send(const StringPiece& strMsg); // 这个函数就包含了只发string信息的情况
    void send(const std::string& message, size_t len); // 向客户端发送消息
    void send(Buffer* buf);  // 直接向对应的fd发送数据, 这个为最终操作.


    // 关闭
    void shutdown(); // 关闭连接
    void forceClose(); // 强制关闭连接
    void forceCloseWithDelay(double seconds); // 强制关闭连接

    // 读
    void startRead(); // 开始接收数据, 由用户调用
    void stopRead(); // 停止接收数据
    bool isReading() const { return isreading; }

    // 设置回调函数
    void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = cb; }
    // 最终会被设置为httpserver的onMessage函数.
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) { m_highWaterMarkCallback = cb; m_highWaterMark = highWaterMark; }
    void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }

    // 展示内部两个buffer
    Buffer* showInputBuffer() { return &m_inputBuffer; }
    Buffer* showOutputBuffer() { return &m_outputBuffer; }

    // 当Tcpserver建立连接和断开连接的时候调用, 调整map
    void connectEstablished();
    void connectDestroyed();

    // 设置相关
    void setTcpNoDelay(bool on);

    void setContext(const HttpContext& ctx) { m_context = ctx; };
    HttpContext* getContext() { return &m_context; };

    bool isTimeCloseing() const { return TimeCloseing; }
    bool TimeCloseing = false;




private:
    enum StateE { // 分别为: 正在连接, 已连接, 已断开, 正在断开
        kConnecting, kConnected, kDisconnected, kDisconnecting
    };
    void setState(StateE s) { m_state = s; }
    const char* stateToString() const;

    void handleRead(Timestamp receiveTime);  // 检查read的返回值后调用相对应的handle函数., 有信息来Messagecb, 对端关闭则handleclose, 出错则handleerror
    void handleWrite();
    void handleClose();
    void handleError();

    // 发送相关, 接收相关, EventLoop相关
    void sendInLoop(const char* msg, size_t len);
    void sendInLoop(const StringPiece& msg);
    void shutdownInLoop();
    void forceCloseInLoop();
    void startReadInLoop();
    void stopReadInLoop();




    EventLoop* m_loop;
    std::string m_name;
    StateE m_state;
    bool isreading;

    std::shared_ptr<Socket> m_socket;
    std::shared_ptr<Channel> m_channel;

    InetAddress m_localAddr;
    InetAddress m_peerAddr;

    ConnectionCallback m_connectionCallback; // 调用tcpserver的connectionCB， 最终调用httpServer的onConnectionCB
    MessageCallback m_messageCallback; // 读到数据之后, 触发的回调
    CloseCallback m_closeCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    WriteCompleteCallback m_writeCompleteCallback;

    size_t m_highWaterMark;

    Buffer m_inputBuffer;
    Buffer m_outputBuffer;

    HttpContext m_context;




};


#endif // TCPCONNECTION_H)
