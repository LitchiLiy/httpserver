#include <buffer.h>
#include <assert.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include <endian.h>
#include <sys/uio.h>
#include <errno.h>
#include <stringPiece.h>
#include <sys/socket.h>






const char Buffer::kCRLF[] = "\r\n";



Buffer::Buffer(size_t initialSize) : // 不能在实现的地方添加默认参数, 在声明的地方添加就可以
    m_buffer(kCheapPrepend + initialSize),
    m_readerIndex(kCheapPrepend),
    m_writerIndex(kCheapPrepend) {
    assert(readableBytesNum() == 0);
    assert(writableBytesNum() == initialSize);
    assert(prependableBytesNum() == kCheapPrepend);
}


void Buffer::swap(Buffer& rhs) {
    m_buffer.swap(rhs.m_buffer);
    std::swap(m_readerIndex, rhs.m_readerIndex);
    std::swap(m_writerIndex, rhs.m_writerIndex);
}


size_t Buffer::readableBytesNum() const {
    return m_writerIndex - m_readerIndex;
}
size_t Buffer::writableBytesNum() const {
    return m_buffer.size() - m_writerIndex;
}
size_t Buffer::prependableBytesNum() const {
    return m_readerIndex;
}


const char* Buffer::peek() const {
    return begin() + m_readerIndex;
}

const char* Buffer::findCRLF() const {
    // search都是const的chari*类型才行, 四个参数
    const char* crlf = std::search(peek(), beginwriteConst(), kCRLF, kCRLF + 2);
    return crlf == beginwriteConst() ? nullptr : crlf;
}
const char* Buffer::findCRLF(const char* start) const {
    const char* crlf = std::search(start, beginwriteConst(), kCRLF, kCRLF + 2);
    return crlf == beginwriteConst() ? nullptr : crlf;
}


const char* Buffer::findEOL() const {
    const void* eol = memchr(peek(), '\n', readableBytesNum()); // 寻找一个字符的位置指针
    return static_cast<const char*>(eol);
}
const char* Buffer::findEOL(const char* start) const {
    const void* eol = memchr(start, '\n', readableBytesNum());
    return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len) {
    assert(len <= readableBytesNum());
    if (len < readableBytesNum()) {
        m_readerIndex += len;
    }
    else {
        retrieveAll();
    }
}
void Buffer::retrieveAll() { // 处理所有的数据, 一次性,
    m_readerIndex = m_writerIndex = kCheapPrepend;
}
void Buffer::retrieveUntil(const char* end) {
    assert(peek() <= end);
    retrieve(end - peek());
}
void Buffer::retrieveInt64() {
    retrieve(sizeof(int64_t));
}
void Buffer::retrieveInt32() {
    retrieve(sizeof(int32_t));
}
void Buffer::retrieveInt16() {
    retrieve(sizeof(int16_t));
}
void Buffer::retrieveInt8() {
    retrieve(sizeof(int8_t));
}

std::string Buffer::retrieveAsString(size_t len) {
    assert(len <= readableBytesNum());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}
std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytesNum());
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytesNum() < len) {
        makeSpace(len);
    }
    assert(writableBytesNum() >= len);
}
char* Buffer::beginwrite() {
    return begin() + m_writerIndex;
}
const char* Buffer::beginwriteConst() const {
    return begin() + m_writerIndex;
}
void Buffer::hasWritten(size_t len) {
    m_writerIndex += len;
}
void Buffer::unwrite(int len) {
    assert(len <= readableBytesNum());
    m_writerIndex -= len;
}

/**
 * @brief buffer供外部调用的最终api, 实现数据向buffer中写入, 并且保证会自动调整内部容量以保证所有数据都被存入.
 *
 * @param data
 * @param len
 */
void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginwrite());
    hasWritten(len);
}
void Buffer::appendInt64(int64_t x) {
    int64_t be64 = htobe64(x);
    append(reinterpret_cast<const char*>(&be64), sizeof(be64));
}
void Buffer::appendInt32(int32_t x) {
    int32_t be32 = htobe32(x);
    append(reinterpret_cast<const char*>(&be32), sizeof(be32));
}
void Buffer::appendInt16(int16_t x) {
    int16_t be16 = htobe16(x);
    append(reinterpret_cast<const char*>(&be16), sizeof(be16));
}
void Buffer::appendInt8(int8_t x) {
    const char data = static_cast<char>(x);
    append(&data, sizeof(data));
}


int64_t Buffer::peekInt64() {
    assert(readableBytesNum() >= sizeof(int64_t));
    int64_t be64 = 0;
    memcpy(&be64, peek(), sizeof(be64));
    return be64;
}
int32_t Buffer::peekInt32() {
    assert(readableBytesNum() >= sizeof(int32_t));
    int32_t be32 = 0;
    memcpy(&be32, peek(), sizeof(be32));
    return be32;
}
int16_t Buffer::peekInt16() {
    assert(readableBytesNum() >= sizeof(int16_t));
    int16_t be16 = 0;
    memcpy(&be16, peek(), sizeof(be16));
    return be16;
}
int8_t Buffer::peekInt8() {
    assert(readableBytesNum() >= sizeof(int8_t));
    char x = 0;
    memcpy(&x, peek(), sizeof(x));
    x = static_cast<int8_t>(x);
    return x;
}

int64_t Buffer::readInt64() {
    int64_t x = peekInt64();
    retrieveInt64();
    return x;
}
int32_t Buffer::readInt32() {
    int32_t x = peekInt32();
    retrieveInt32();
    return x;
}
int16_t Buffer::readInt16() {
    int16_t x = peekInt16();
    retrieveInt16();
    return x;
}
int8_t Buffer::readInt8() {
    int8_t x = peekInt8();
    retrieveInt8();
    return x;
}

void Buffer::prepend(const void* data, size_t len) {
    assert(len <= prependableBytesNum());
    m_readerIndex -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + m_readerIndex);
}
void Buffer::prependInt8(int8_t x) {
    prepend(&x, sizeof(x));
}
void Buffer::prependInt16(int16_t x) {
    prepend(&x, sizeof(x));
}
void Buffer::prependInt32(int32_t x) {
    prepend(&x, sizeof(x));
}
void Buffer::prependInt64(int64_t x) {
    prepend(&x, sizeof(x));
}

/**
 * @brief ET模式的读取, 直到所有数据读完, 测试成功可行
 *
 * @param fd
 * @param savedErrno
 * @return ssize_t
 */
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    ssize_t n = 0;
    int flag = 1;
    char buf[10];
    memset(buf, 0, sizeof(buf));
    int ret = 0;
    while (flag) {
        ret = 0;
        ret = recv(fd, buf, sizeof(buf), 0); // 0为默认模式, fd是非阻塞的在Acceptor里面可以看到, 故这里也是非阻塞的
        if (ret > 0) {
            // 收到数据了
            if (ret != sizeof(buf)) {
                // 收到数据了, 但是不够
                buf[ret] = '\0';
            }
            n += ret;
            append(buf, ret);
            memset(buf, 0, sizeof(buf));
        }
        else if (ret == 0) {
            // 对方关闭了
            flag = 0;
            *savedErrno = errno;
        }
        else if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 对方没有数据了,跳出
                flag = 0;
                *savedErrno = errno;
            }
            else {
                // 错误了, 跳出
                flag = 0;
                *savedErrno = errno;
            }
        }
    }
    return n;
}
// ssize_t Buffer::readFd(int fd, int* savedErrno) {
//     char buf[65536];
//     struct iovec vec[2]; // 内部为buff指针和buff大小两个结构
//     const size_t writeable = writableBytesNum();
//     vec[0].iov_base = begin() + m_writerIndex;
//     vec[0].iov_len = writeable;
//     vec[1].iov_base = buf;
//     vec[1].iov_len = sizeof(buf);
//     ssize_t n = readv(fd, vec, 2); // 123分别为fd, iovec指针, iovec大小, 意思就是从iovec第0个开始, 读取数据从0开始填满, 直到3为止, 填不足就不填了, 在库函数uio.h中.
//     if (n < 0) {
//         *savedErrno = errno;
//     }
//     else if (static_cast<size_t>(n) <= writeable) {
//         m_writerIndex += n;
//     }
//     else {
//         m_writerIndex = m_buffer.size();
//         append(buf, n - writeable);
//     }
//     return n;
// }

void Buffer::makeSpace(size_t len) {
    if (writableBytesNum() + prependableBytesNum() < len + kCheapPrepend)  // 不能把预留的前置算进去
    {
        m_buffer.resize(writableBytesNum() + len); // 直接暴力增加, 这样可以直接按着writeindex写下去了
    }
    else {
        assert(kCheapPrepend < m_readerIndex);
        size_t readable = readableBytesNum();
        std::copy(begin() + m_readerIndex, begin() + m_writerIndex, begin() + kCheapPrepend); // 不能把预留的前置加上
        m_readerIndex = kCheapPrepend;
        m_writerIndex = m_readerIndex + readable;
        assert(readable == readableBytesNum());
    }
}