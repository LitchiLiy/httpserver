#include <httpServer.h>
#include <logging.h>
#include <httpContext.h>
#include <httpRequest.h>
#include <httpResponse.h>
#include <callBacks.h>
#include <tcpConnection.h>
#include <eventLoop.h>

int longConnectionTime = 0;

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


/**
 * @brief 对于httpServer类来说, 最重要的函数就是setHttpCB函数,
 *
 * @param Loop
 * @param listenAddr
 * @param name
 * @param option
 */
HttpServer::HttpServer(EventLoop* Loop,
                       const InetAddress& listenAddr,
                       const string& name,
                       TcpServer::Option option) :
    server_(Loop, listenAddr, name, option),
    httpCallback_(defaultHttpCallback) {
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));  // 这两部只是设置了应对之策而已

    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}

void HttpServer::start() {
    server_.start();
}

/**
 * @brief 检查一下Tcp是否已经连接了, 如果链接了, 那就为这个链接配置上下文, 这个onConnection被tcpServer的onConCb指针保存， 然后再被conn的onConCb保存
 *
 * @param conn
 */
void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        conn->setContext(HttpContext()); // 构造函数会输出一个类实例赋值进去
    }
}

/**
 * @brief 这是一个HttpServer的收到信息后的应对函数, 收到的请求放在buf中, 然后调用这个函数就可以实现: 1. 调用tcpconnection的ctx来解析请求, 然后将信息存入req中. 2. 如果全部解析完成了, 就调用onRequest, 并重置tcpConnectiion的ctx, 这个函数最终会被设置为TcpConnection的Mesg回调函数
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
        ctx->reset(); // 清空ctx
    }
}


/**
 * @brief 这个函数会被onMsg调用, 而onMsg会被conn的回调函数调用. 而这个函数会调用httpcb函数, 所以设置httpcb很重要. httbcb函数就是用来写response用的. 相当于状态机
 *
 * @param conn 这个i连接就是客户但的连接, 放在这里纯粹是为了使用连接的seng来发送消息
 * @param req  这个req就是来自conn的ctx的req, 从noMsg代码中可以看出
 */
void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
    const string& connection = req.getHeader("Connection");
    bool close = (connection == "close" || (req.showVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive")); // http1.0是默认close的, 而http1.1是默认keep的

    HttpResponse Response(close); // 这个close会被下面的回调所修正,不用担心他的影响
    httpCallback_(req, &Response);// 调用response回调
    // 设置长连接定时器
    if (!Response.isCloseLive() && !conn->isTimeCloseing()) {
        // if (!Response.isCloseLive()) {
        auto lp = conn->getLoop();
        // conn->TimeCloseing = true;
        // 每次得到的seq都不同
        auto tmp = conn->timerid_;
        conn->timerid_ = lp->runAfter(longConnectionTime, std::bind(&TcpConnection::connectDestroyed, conn)); // 设置60s的定时器长连接销毁, 用shutdown总会出毛病, 不知道为啥
        if (tmp.showTimer() != nullptr) lp->cancelTimer(tmp); // 如果不为空, condest触发了会使得这里为空
    }
    Buffer buf;
    Response.appendToBuffer(&buf);
    conn->send(&buf);
    if (Response.closeConnection()) {
        conn->shutdown(); // 这只是关闭我们这边的写而已
    }
}

