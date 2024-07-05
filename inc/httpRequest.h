#if !defined(HTTPREQUEST_H)
#define HTTPREQUEST_H

#include <string>
#include <timestamp.h>
#include <map>
#include <ctype.h>




using namespace std;

/*
一个基层信息类, 只存储信息, 不处理数据.
*/






class HttpRequest
{
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    enum Version {
        kUnknown, kHttp10, kHttp11
    };


public:
    // HttpRequest() = default;
    ~HttpRequest() = default;
    HttpRequest() :method_(kInvalid), version_(kUnknown) {}

public:
    /*
        这些函数的参数都是输入两个const char*的指针, 一个表示开头, 一个表示结尾, 也就是说传入的都是已经截好了的, 而不是原始的头部, 而是已经截好了的字符串. 甚至连接收时间都是外面输入进来的. 只存数据

        都是左闭右开
    */

    bool setMethod(const char* begin, const char* end) {
        string str(begin, end);
        if (str == "GET") {
            method_ = kGet;
        }
        else if (str == "POST") {
            method_ = kPost;
        }
        else if (str == "HEAD") {
            method_ = kHead;
        }
        else if (str == "PUT") {
            method_ = kPut;
        }
        else if (str == "DELETE") {
            method_ = kDelete;
        }
        else {
            method_ = kInvalid;
            return false;
        }
        return true;
    }

    Method showMethod() const { return method_; }
    const char* methodString() const {
        const char* ret = "UNKNOWN";
        switch (method_) {
        case kGet:
            ret = "GET";
            break;
        case kPost:
            ret = "POST";
            break;
        case kHead:
            ret = "HEAD";
            break;
        case kPut:
            ret = "PUT";
            break;
        case kDelete:
            ret = "DELETE";
            break;
        default:
            break;
        }
        return ret;
    }

    void setVersion(Version v) {
        version_ = v;
    }

    Version showVersion()  const {
        return version_;
    }

    void setPath(const char* begin, const char* end) {
        path_.assign(begin, end);
    }
    const string& path() const { return path_; }

    void setQuery(const char* begin, const char* end) {
        query_.assign(begin, end);
    }
    const string& query() const { return query_; }

    void setReceiveTime(Timestamp t) { recvTstamp_ = t; }
    Timestamp receiveTime() const { return recvTstamp_; }

    // 这个函数是输入一行比如Host: www.baidu.com  , begin指向H, colon指向:, end指向末尾, 这一行的作用就是分出Host : www.baidu.com这三部分添加到头部map中.
    void addHeader(const char* begin, const char* colon, const char* end) {
        string field(begin, colon);
        ++colon;
        while (colon < end && isspace(*colon)) {  // 识别空白字符, 空格, 制表符, 垂直制表符, 换行符, 回车符, 换页符, 即 \t \v \n \r \f
            ++colon;
        }
        string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }
    // 输入一个头部, 从map中找出对应的输出
    string getHeader(const string& field) const {
        string ret;
        auto it = headers_.find(field);
        if (it != headers_.end()) {
            ret = it->second;
        }
        return ret;
    }

    const map<string, string>& headers() const { return headers_; }

    void swap(HttpRequest& that) {
        using std::swap;
        swap(method_, that.method_);
        swap(version_, that.version_);
        swap(path_, that.path_);
        swap(query_, that.query_);
        swap(recvTstamp_, that.recvTstamp_);
        swap(headers_, that.headers_);
    }


private:
    /*
        私有变量有: 请求方式, 版本号, URL路径, query搜索string, 接收时间戳, 一个头部map
    */
    Method method_;
    Version version_;
    string path_;
    string query_;
    Timestamp recvTstamp_;
    std::map<string, string> headers_;
};



#endif // HTTPREQUEST_H)
