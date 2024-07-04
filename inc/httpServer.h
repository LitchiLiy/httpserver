#if !defined(HTTPSERVER_H)
#define HTTPSERVER_H

#include <tcpServer.h>


class HttpResponse;
class HttpRequest;
/// @brief 一个HttpServer实例, 里面保存着TcpServer实例, TcpServer只是建立连接, 在次基础上再构建Http服务器
class HttpServer {
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
    void onMessage(cosnt TcpConnectionPtr& conn,
        Buffer* buf,
        Timestamp receiveTime);
    void onRequest(const HttpRequest& req, HttpResponse* resp);

private:
    TcpServer server_;
    HttpCallback httpCallback_;
};



#endif // HTTPSERVER_H)
