/*
    这个用来测试httpserver是否有用
*/

#include <httpServer.h>
#include <httpRequest.h>
#include <httpResponse.h>
#include <eventLoop.h>
#include <logging.h>
#include <mInetAddress.h>
#include <iostream>
#include <map>
#include <string>
#include <asyncLogging.h>
#include <logging.h>




using namespace std;


bool benchmark = false;


void onRequest(const HttpRequest& req, HttpResponse* resp) {
    LOG_INFO << "Headers " << req.methodString() << " " << req.path();
    if (!benchmark) {
        const std::map<string, string>& headers = req.headers();
        for (const auto& header : headers) {
            LOG_INFO << header.first << ": " << header.second;
        }
    }
    // std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
    // if (!benchmark) {
    //     const std::map<string, string>& headers = req.headers();
    //     for (const auto& header : headers) {
    //         std::cout << header.first << ": " << header.second << std::endl;
    //     }
    // }

    const string body = req.getPathhtml();
    if (body != "404") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "litchily");
        resp->setbody(body);

    }
    else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

// 日志同步线程
std::shared_ptr<AsyncLogging> g_asyncLog;
void asyncOutput(const char* msg, int len) {
    g_asyncLog->append(msg, len);
}




int main(int argc, char* argv[]) {
    // 日志同步线程 
    char name[256] = { '\0' };  // 文件名
    char arr[] = { "../log/Logfile" };   // 直接本地文件名
    strncpy(name, arr, sizeof name - 1);
    g_asyncLog = std::make_shared<AsyncLogging>(name, 50 * 1024 * 1024);
    g_asyncLog->start();  // 构建一个新线程, 不管主线程的事情
    Logger::setOutput(asyncOutput);

    int numThreads = 2;
    if (argc > 1) {
        benchmark = true;
        Logger::setLogLevel(Logger::WARN);
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    loop.setMainEventLoop();
    // HttpServer server(&loop, InetAddress("172.19.46.27", 8000, false), "litchi");
    HttpServer server(&loop, InetAddress("0.0.0.0", 8000, false), "litchi");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();

    return 0;
}
