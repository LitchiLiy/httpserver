#if !defined(HTTPRESPONSE_H)
#define HTTPRESPONSE_H



#include <string>

#include <map>


using namespace std;
class Buffer;

/// @brief http响应信息存储类, 大概的意思就是你想写一个相应报文回去, 你可以先用这个类把想要传出的信息全部写道这个类里面来, 然后调用appendBuffer函数写入到buf中.
class HttpResponse {
public:
    HttpResponse() = default;
    ~HttpResponse() = default;
    explicit HttpResponse(bool close) :
        statusCode_(kUnknown),
        closeConnection_(close) {}

    enum HttpStatusCode {
        kUnknown, k200Ok = 200, k301MovedPermanently = 301, k400BadRequest = 400, k404NotFound = 404,
    };
private:
    HttpStatusCode statusCode_;
    bool closeConnection_; // 是否短链接
    std::map<string, string> headers_;
    string statusMessage_;
    string body_;

public:
    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
    void setCloseConnection(bool on) { closeConnection_ = on; }
    void setStatusMessage(const string& message) { statusMessage_ = message; }
    bool closeConnection() const { return closeConnection_; }

    /// @brief 针对Content-Type特地设置了一个函数
    /// @param contentType 
    void setContentType(const string& contentType) { headers_["Content-Type"] = contentType; }

    /// @brief 设置响应头和相应体
    /// @param key 
    /// @param value 
    void addHeader(const string& key, const string& value) { headers_[key] = value; }

    void setbody(const string& body) { body_ = body; }

    void appendToBuffer(Buffer* buf) const;
};





#endif // HTTPRESPONSE_H)   

