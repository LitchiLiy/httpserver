#if !defined(BUFFER_H)
#define BUFFER_H

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <stringPiece.h>



/*
这就是一个纯粹的缓冲区类, 缓冲区被两条线划分为三个区间, 分别是前置区域, 未读区域和可写区域:
1. 前置区域,表示已经读走不需要的数据但仍然留在vec中
2. 未读区域, 未读走的数据
3. 可写区域, 空区域

还有两条线
1. readerIndex: 区分前置与可读， 从idx位置就是没有读走的。
2. writerIndex: 区分可写与未写， 从可写index开始写入数据

还有从fd读取数据到本地的功能:
1. append, 直接插入, 不用考虑任何事情
2. readint64, 直接读, 不用考虑任何事情

*/
class StringPiece;



class Buffer {
public:
    static const size_t kInitialSize = 1024;
    static const size_t kCheapPrepend = 8; // 预留空间, k为const, C为低成本前置的意思
public:
    // Buffer() = default;
    // ~Buffer() = default;
    explicit Buffer(size_t initialSize = kInitialSize);

    void swap(Buffer& rhs);
    size_t readableBytesNum() const;
    size_t writableBytesNum() const;
    size_t prependableBytesNum() const;

    const char* peek() const; // 可读起始位置.

    // 从buf中找东西, 
    const char* findCRLF() const;
    const char* findCRLF(const char* start) const;

    // 找EOL
    const char* findEOL() const;
    const char* findEOL(const char* start) const;

    // 数据检索相关, 检索过后相当于这数据就已经被清除掉了, 所以read指针往前走, 调用任何retrieve之前需要保证数据确实已经被处理了, 这只是一个移动操作.
    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveInt64(); // 处理一个int64大小的数据量
    void retrieveInt32(); // 处理一个int32大小的数据量
    void retrieveInt16(); // 处理一个int16大小的数据量
    void retrieveInt8(); // 处理一个int8大小的数据量
    void retrieveAll();

    std::string retrieveAsString(size_t len); // 从可读的peek开始, 处理len长度, 然后返回处理的数据string
    std::string retrieveAllAsString();

    // 关于写入
    void ensureWritableBytes(size_t len); // 任何写操作之前都要先调用这个函数, 确保可写的长度足够, 从代码上来看, 只有不够的时候才会调用makeSpace
    char* beginwrite(); // 返回可写起始位置在vec中的char*的ptr
    const char* beginwriteConst() const;
    void hasWritten(size_t len); // 将可写idx推进len
    void unwrite(int len); // 将可写idx回退len

    void append(const char* data, size_t len); // 讲data的数据写入本vec中, 直接调用即可, 结果就是任何情况下都能将数据存入vec中.
    void append(const StringPiece& str)
    {
        append(str.data(), str.length());
    }
    void append(const void* /*restrict*/ data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }
    void appendInt64(int64_t x); // 都是要转换成网络字节序的
    void appendInt32(int32_t x);
    void appendInt16(int16_t x);
    void appendInt8(int8_t x);

    // 读取数据
    int64_t peekInt64(); // 从可读index处读取一个int64, 转换成主机字节序后返回, 只读, 不移动idx
    int32_t peekInt32();
    int16_t peekInt16();
    int8_t peekInt8();

    // 读取管理, 包括读取数据, 移动已读, 读用peek, 移动用retrieve, 直接调用即可, 结果就是任何情况下都能将数据读取出来并保证buffer的完整性
    int64_t readInt64();
    int32_t readInt32();
    int16_t readInt16();
    int8_t readInt8();

    // 将数据插入前置位置, 这里指的前置位置是已读idx的前面len长度.
    void prepend(const void* data, size_t len); // 将数据插入前置位置, 还会移动idx. 往前移
    void prependInt8(int8_t x);
    void prependInt16(int16_t x);
    void prependInt32(int32_t x);
    void prependInt64(int64_t x);

    // 关心vector
    ssize_t internalCapacity() const { return m_buffer.capacity(); }
    ssize_t readFd(int fd, int* savedErrno);  // 从fd中读取数据到buffer内部的vec中.




private:
    void makeSpace(size_t len); // 判断空间是否够大装下len, 不够就扩容, 够就调整数据在vector的位置, 然后挪动数据. 此函数主要的目的是调整数据位置.
    char* begin() { return &*m_buffer.begin(); }
    const char* begin() const { return &*m_buffer.begin(); }


private:
    std::vector<char> m_buffer;
    size_t m_readerIndex;
    size_t m_writerIndex;

    static const char kCRLF[];
};

#endif // BUFFER_H
