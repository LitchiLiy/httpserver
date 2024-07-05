#include <httpServer.h>
#include <logging.h>
#include <httpContext.h>
#include <httpRequest.h>
#include <httpResponse.h>
#include <callBacks.h>
#include <tcpConnection.h>

/**
 * @brief 提供一个默认的httpserver的respoones模板， 就是404
 *
 * @param resp
 */
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found!");
    resp->setCloseConnection(true);
}



HttpServer::HttpServer(EventLoop* Loop,
                       const InetAddress& listenAddr,
                       const string& name,
                       TcpServer::Option option) :
    server_(Loop, listenAddr, name, option),
    httpCallback_(defaultHttpCallback) {
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start() {
    server_.start();
}

/**
 * @brief 检查一下Tcp是否已经连接了, 如果链接了, 那就为这个链接配置上下文
 *
 * @param conn
 */
void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        conn->setContext(HttpContext()); // 构造函数会输出一个类实例赋值进去
    }
}

/**
 * @brief 这是一个HttpServer的收到信息后的应对函数, 收到的请求放在buf中, 然后调用这个函数就可以实现: 1. 调用tcpconnection的ctx来解析请求, 然后将信息存入req中. 2. 如果全部解析完成了, 就调用onRequest, 并重置tcpConnectiion的ctx
 *
 *
 * @param conn tcpconnection指针
 * @param buf  收到的请求信息全部存在buf中
 * @param receiveTime
 */
void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime) {
    HttpContext* ctx = conn->getContext();
    if (!ctx->parseRequest(buf, receiveTime)) {  // 如果解析失败, 就发送400
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (ctx->gotAll()) {
        onRequest(conn, ctx->request()); // 返回请求的存储信息类
        ctx->reset();
    }
}



void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
    const string& connection = req.getHeader("Connection");
    bool close = (connection == "close" || (req.showVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive"));

    HttpResponse Response(close);
    httpCallback_(req, &Response);// 调用response回调
    Buffer buf;
    Response.appendToBuffer(&buf);
    conn->send(&buf);
    if (Response.closeConnection()) {
        conn->shutdown();
    }
}