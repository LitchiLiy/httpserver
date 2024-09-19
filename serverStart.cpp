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

extern string pollmode;
extern bool isshowTerminal;
extern int longConnectionTime;

bool closealive = true;
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
    // 获取html文件
    const string body = req.getPathhtml();
    if (body != "404") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "litchily");
        resp->setbody(body);
        resp->setCloseConnection(closealive);  // 这里如果没找到就关闭链接返回 Connection: close, 但是为了维持长连接， 我这里设置了false
    }
    else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        // resp->setCloseConnection(false);  // 这里如果没找到就关闭链接返回 Connection: close, 但是为了维持长连接， 我这里设置了false
        resp->setCloseConnection(closealive); // 如果想设置短连接, 这里设置为true就行了, 后面会根据回复的close, 服务器这边主动关闭链接或者60s后服务器关闭连接
    }
}

// 日志同步线程
std::shared_ptr<AsyncLogging> g_asyncLog;
void asyncOutput(const char* msg, int len) {
    g_asyncLog->append(msg, len);
}


int main(int argc, char* argv[]) {
    // argv[0] = "serverStart";
    // argv[1] = poll方式;
    if (argc > 1) {
        if (strcmp(argv[1], "epoll") == 0) {
            pollmode = "epoll";
        }
        else if (strcmp(argv[1], "select") == 0) {
            pollmode = "select";
        }
    }
    if (pollmode == "") {
        pollmode = "epoll";
    }



    // 日志同步线程 
    char name[256] = { '\0' };  // 文件名
    char arr[] = { "../log/Logfile" };   // 直接本地文件名
    strncpy(name, arr, sizeof name - 1);
    // g_asyncLog = std::make_shared<AsyncLogging>(name, 50 * 1024 * 1024);
    g_asyncLog = std::shared_ptr<AsyncLogging>(AsyncLogging::getAsyncLog(name, 500 * 1024 * 1024)); // 这里文件大小大约50Mbit
    g_asyncLog->start();  // 构建一个新线程, 不管主线程的事情
    Logger::setOutput(asyncOutput);

    int numThreads = 10;
    if (argc > 1) {
        benchmark = true;
        Logger::setLogLevel(Logger::WARN);
        numThreads = atoi(argv[1]);
    }


    EventLoop loop(pollmode);
    loop.setMainEventLoop();
    closealive = true; // 是否断开
    isshowTerminal = true; // 是否在终端展示
    longConnectionTime = 60;  // 长连接定时器

    // LOG_INFO << "服务器地址为" << "http://172.19.46.27:14789";
    // HttpServer server(&loop, InetAddress("172.24.42.9", 8888, false), "litchi");
    HttpServer server(&loop, InetAddress("0.0.0.0", 14789, false), "litchi");
    // HttpServer server(&loop, InetAddress("172.19.46.27", 8888, false), "litchi"); // wsl地址
    // http://172.19.46.27:14789
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();

    return 0;
}

