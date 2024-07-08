#include <httpContext.h>
#include <buffer.h>
#include <httpRequest.h>
#include <timestamp.h>




// 处理请求行, 收到请求信息之后, 第一步先处理请求行, 将请求方法, 资源路径, 版本号存储起来, 返回结果就是成功true, 或者失败false
bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            // 查询字符串, 格式为URL?q=muduo&role=admit 类似这样, 表示关键词为moduo, 权限为管理员权限, 前面为资源路径3, 后面为查询字符串
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.setPath(start, question);
                request_.setQuery(question + 1, space);
            }
            else {
                request_.setPath(start, space);
            }

            // 最后获得版本号
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else {
                    request_.setVersion(HttpRequest::kHttp10);
                }
            }
            else {
                succeed = false;
            }
        }
    }
    return succeed;
}


// 当请求报文被buf全部接收后, 调用这个函数来解析
/**
 * @brief 当你接收到buf之后, 然后就调用httpContext函数的这个api来解析buf的请求报文, 这个函数会将内容存储到本类的成员变狼request中.
 *
 * @param buf
 * @param receiveTime
 * @return true
 * @return false
 */
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2); // 解析就可以了, 不需要把buf中的资料删掉, 意思就是移动可读指针
                    state_ = kExpectHeaders;
                }
                else {
                    hasMore = false;
                }
            }
            else {
                hasMore = false;
            }
        }
        else if (state_ == kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else {
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            }
            else {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody) {
            {
                // 这里处理请求体
            }
        }
    }
    return ok;
}

