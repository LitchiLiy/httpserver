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


using namespace std;


bool benchmark = false;


void onRequest(const HttpRequest& req, HttpResponse* resp) {
    std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
    if (!benchmark) {
        const std::map<string, string>& headers = req.headers();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

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


    // if (req.path() == "/") {
    //     resp->setStatusCode(HttpResponse::k200Ok);
    //     resp->setStatusMessage("OK");
    //     resp->setContentType("text/html");
    //     resp->addHeader("Server", "litchily");
    //     string now = Timestamp::now().toFormatString();
    //     resp->setbody("<html><head><title>This is title</title></head>"
    //                   "<body><h1>Hello</h1>Now is " + now +
    //                   "</body></html>");
    // }
    // else if (req.path() == "/favicon.ico") {
    //     resp->setStatusCode(HttpResponse::k200Ok);
    //     resp->setStatusMessage("OK");
    //     resp->setContentType("image/png");
    //     resp->setbody(string(favicon, sizeof favicon));
    // }
    // else if (req.path() == "/hello") {
    //     resp->setStatusCode(HttpResponse::k200Ok);
    //     resp->setStatusMessage("OK");
    //     resp->setContentType("text/plain");
    //     resp->addHeader("Server", "Muduo");
    //     resp->setbody("hello, world!\n");
    // }
    // else {
    //     resp->setStatusCode(HttpResponse::k404NotFound);
    //     resp->setStatusMessage("Not Found");
    //     resp->setCloseConnection(true);
    // }
}

int main(int argc, char* argv[]) {
    int numThreads = 2;
    if (argc > 1) {
        benchmark = true;
        Logger::setLogLevel(Logger::WARN);
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    loop.setMainEventLoop();
    HttpServer server(&loop, InetAddress("172.19.46.27", 8000, false), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();

    return 0;
}

