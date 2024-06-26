#if !defined(MACCEPTOR_H)
#define MACCEPTOR_H

/*
作用, 已知socket已经实现了关于sock之类的功能, 还保存着一个fd, 但是没有构建sock实例, address用来处理任何与地址有关的操作, 这个Accept就是用来处理任何有关accept的操作, 包括一个channel, 一个fd, 一个回调函数也就是handleRead函数. 还要处理新连接的时候的逻辑
1. channel类实例
2. 因为有channel, 所以有eventloop*指针.
3. 一个Socket类, 用来掌管对应的socket
4. 一个回调函数

也就是说, 我给你一个InetAddress这个类实例, 也就是给你服务器地址, 让这个accept类接管从创建fd到接收数据的整个流程
*/

#include <mSocket.h>
#include <channel.h>

class EventLoop;
class InetAddress;

class Acceptor {
    typedef std::function<void(int, const InetAddress&)> newConnectionCallback_f;
public:
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    ~Acceptor();



    bool islistening() const { return isListening; }
    void listen();

    void setNewConnectionCallback(const newConnectionCallback_f& cb) { m_newConnectionCallback = cb; }

private:
    void handleRead();

    int m_acceptFd;

    bool isListening;

    Channel m_channel;
    EventLoop* m_loop;
    Socket m_socket;
    newConnectionCallback_f m_newConnectionCallback;
};


#endif // MACCEPTOR_H)
