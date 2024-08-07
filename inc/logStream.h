#if !defined(LOGSTREAM_H)   
#define LOGSTREAM_H

#include <string.h>
#include <stdio.h>
#include <stringPiece.h>

#include <assert.h>

/*
FixedBuffer就是记录数据用的, 当作一个纯纯的buffer, 有添加, 重置, toString等方法, 大概的用法就是, 直接调用append, 这个append会先检查大小是否足够装, 不够就不装. 够就装

对于LogStream类来说, 他只有一个buffer, 功能就是定义了这个类的<<操作符.

总功能:1. append就是简单的调用buffer的append, 2. <<操作符就是用format函数写入, 但是要注意超出buffer则跳过.
*/



using namespace std;


const long long kSmallBuffer = 1024 * 1024 / 8;
const long long kLargeBuffer = 1024 * 1024 * 1024 / 8;

template<long long SIZE>
class FixedBuffer {
public:
    FixedBuffer() :m_cur(m_data) {

    }
    ~FixedBuffer() {

    }

    void append(const char* buf, size_t len) {
        if (avail() > len) {
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }
    const char* data() const { return m_data; }
    int length() const { return static_cast<int>(m_cur - m_data); }
    char* current() { return m_cur; }
    int avail() const {
        auto ret = end() - m_cur; // 正常ret为4000
        return static_cast<int>(ret);
    } // 检查剩余空间是否还有剩下的
    void add(size_t len) { m_cur += len; }

    void reset() { m_cur = m_data; }
    void bzero() { memset(m_data, 0, sizeof(m_data)); }

    string toString() const { return string(m_data, m_cur); }
    StringPiece toStringPiece() const { return StringPiece(m_data, m_cur - m_data); }

private:
    const char* end() const { return m_data + sizeof(m_data); }

private:
    char* m_cur; // 指向可写位置的指针
    char m_data[SIZE];
    void (*m_cookie)() = nullptr;
};





class LogStream {
    typedef LogStream self;
public:
    typedef FixedBuffer<kSmallBuffer> Buffer; // 一个char为一个字节, 4000个字节为一个kSmall, 也就是一个日志32kb.
    LogStream() = default;
    ~LogStream() = default;

    self& operator<<(bool v);
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(float v);
    self& operator<<(double v);
    self& operator<<(char v);
    self& operator<<(const void* p);
    self& operator<<(const char* v);
    self& operator<<(const std::string& v);
    self& operator<<(const Buffer& v) {
        *this << v.toStringPiece();
        return *this;
    }
    self& operator<<(const StringPiece& v);

    void append(const char* data, int len) { m_buffer.append(data, len); }
    const Buffer& buffer() const { return m_buffer; }
    void resetBuffer() { m_buffer.reset(); }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T); // <<符号就会调用这个函数, 将数据写入buffer中, 但是超出范围不会执行写入, 而是跳过

    Buffer m_buffer;
    static const int kMaxNumericSize = 48;
};



class Fmt // : noncopyable
{
public:
    template<typename T>
    Fmt(const char* fmt, T val) {
        // static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

        length_ = snprintf(buf_, sizeof buf_, fmt, val);
        assert(static_cast<size_t>(length_) < sizeof buf_);
    }

    const char* data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};



inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
    s.append(fmt.data(), fmt.length());
    return s;
}




#endif // LOGSTREAM_H)  
