#if !defined(HTTPSERVER_H)
#define HTTPSERVER_H

/*
    提供http的一些服务, 是中间层, 内部没有内置默认应答CB, 需要在构建httpServer实例之后再设置一个应答CB, 用来标准回复每一种请求. 或者说, HttpServer类把response, request, tcpserver类之间的配合整合起来了.
*/




#include <tcpServer.h>

class HttpResponse;
class HttpRequest;
/// @brief 一个HttpServer实例, 里面保存着TcpServer实例, TcpServer只是建立连接, 在次基础上再构建Http服务器
class HttpServer
{
public:
    typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& name,
               TcpServer::Option option = TcpServer::kNoReusePort);

    EventLoop* getLoop() const { return server_.getLoop(); }
    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }
    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

private:
    TcpServer server_;
    HttpCallback httpCallback_; // 这个东西的作用就是: 客户端发来请求报文, 处理成req类之后, 调用这个回调, 根据req来填写response. 作用就是填写response
};

#endif // HTTPSERVER_H
