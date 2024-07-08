#include <logStream.h>
#include <stringPiece.h>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <limits>
#include <cstdint>




using namespace std;

const char digits[] = "9876543210123456789";
const char digitsHex[] = "0123456789ABCDEF";
const char* zero = digits + 9;



// 确保kMax对double这些类型的最大有效数字位还要大.
void LogStream::staticCheck() {
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,
                  "kMaxNumericSize is large enough");
}


// 将数字形式转换成字符串添加到buffer中.
template<typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

// 将数字形式转换成十六进制字符串添加到buffer中
size_t convertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}


template<typename T>
void LogStream::formatInteger(T v) {
    if (m_buffer.avail() >= kMaxNumericSize) {
        size_t len = convert(m_buffer.current(), v);
        m_buffer.add(len);
    }
}


LogStream& LogStream::operator<<(bool v) {
    m_buffer.append(v ? "1" : "0", 1);
    return *this;
}

// int 类型
LogStream& LogStream::operator<<(int v) {
    formatInteger(v);   // 把int数据插入buffer中.
    return *this;
}

// short和int类型是一样的, 这里直接用<<int
LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}


LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}


LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}


LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}


// .12g中12表示展示有效数字为12位, 而g表示在%e科学计数法和%f浮点数中智能做出选择.
LogStream& LogStream::operator<<(double v) {
    if (m_buffer.avail() >= kMaxNumericSize) {
        int len = snprintf(m_buffer.current(), kMaxNumericSize, "%.12g", v);
        m_buffer.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (m_buffer.avail() >= kMaxNumericSize) {
        char* buf = m_buffer.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        m_buffer.add(len + 2);
    }
    return *this;
}


LogStream& LogStream::operator<<(char v) {
    m_buffer.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char* v) {
    if (v) {
        m_buffer.append(v, strlen(v));
    }
    else {
        m_buffer.append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const std::string& v) {
    m_buffer.append(v.c_str(), v.size());
    return *this;
}


LogStream& LogStream::operator<<(const StringPiece& v) {
    m_buffer.append(v.data(), v.length());
    return *this;
}

