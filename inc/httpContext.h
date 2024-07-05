#if !defined(HTTPCONTEXT_H)
#define HTTPCONTEXT_H



#include <httpRequest.h>

/*
这个类会从http客户端发来的请求中将数据解析出来, 然后构造httpRequest对象来存储, 这个过程会经历不同的状态, 最终完成解析或者出错.

也就是说这个类只负责处理, 不负责接收, 接收到buffer之后, 再用这个类去处理,

内部只保存一个httpRequest对象, 也就是一个HttpContext只处理一次请求或发送
*/

class Buffer;
class Timestamp;

class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine, kExpectHeaders, kExpectBody, kGotAll
    };

    // HttpContext() = default;
    ~HttpContext() = default;
    HttpContext() : state_(kExpectRequestLine) {}

public:
    bool parseRequest(Buffer* buf, Timestamp receiveTime);   // 核心函数

    bool gotAll() const {
        return state_ == kGotAll;
    }
    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }
    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);
    HttpRequestParseState state_;
    HttpRequest request_;
};



#endif // HTTPCONTEXT_H)