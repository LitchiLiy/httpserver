#include <tcpConnection.h>
#include <eventLoop.h>
#include <mSocket.h>
#include <channel.h>
#include <functional>
#include <stringPiece.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <logging.h>
#include <timerId.h>



using namespace std;

/**
 * @brief Tcp的构造函数
 *
 * @param loop
 * @param name
 * @param sockfd 可以看到sockfd其实是TcpConnection外部创建的, 我们这里只是用来保存socket的信息而已, 而且方便做其他的事情
 * @param localAddr
 * @param peerAddr
 */
TcpConnection::TcpConnection(EventLoop* loop, const string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr) :
    m_loop(loop),
    m_name(name),
    m_socket(new Socket(sockfd)),
    m_channel(new Channel(loop, sockfd)),
    m_localAddr(localAddr),
    m_peerAddr(peerAddr),
    m_state(kConnecting), // 这表明构建这个实例的时候是正在连接的时候
    isreading(true),
    m_highWaterMark(64 * 1024 * 1024),
    TimeCloseing(false),
    timerid_(nullptr, -1),
    m_inputBuffer(1024) {
    m_channel->setReadCallBack(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)); // functional里面的东西

    m_channel->setWriteCallBack(
        std::bind(&TcpConnection::handleWrite, this));

    m_channel->setCloseCallBakc(
        std::bind(&TcpConnection::handleClose, this));

    m_channel->setErroCallBack(
        std::bind(&TcpConnection::handleError, this));

    m_socket->setKeepAlive(true);

}


TcpConnection::~TcpConnection() {
    assert(m_state == kDisconnected);
}

// 发送相关+
// 总发送函数, 给一个date的指针, 长度, 将他发到对应fd里面去, 如果忙, 就存入outputBuffer, 如果超出高水位, 就触发高水位回调. 供send调用, send才是面对用户的渠道
void TcpConnection::sendInLoop(const char* msg, size_t len) {
    // 凡是InLoop里面的都是在这里面调用的
    m_loop->isInLoopThread();

    ssize_t sendedNum = 0; // 已经写了
    size_t remaining = len;
    bool faultError = false;

    if (m_state == kDisconnected) {
        cout << "this TcpConnection is disconnected" << endl;
    }

    // channel已经输出使能了, 并且buffer没有东西
    if (!m_channel->isWriting() && m_outputBuffer.readableBytesNum() == 0) {
        sendedNum = write(m_socket->fd(), msg, len);
        if (sendedNum > 0) {
            remaining = len - sendedNum;
            // 如果发完了, 那么就调用发完回调. 直接放入排队.
            if (remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, make_shared<TcpConnection>(*this)));
            }
        }
        else {
            // 没发完, 看看是不是对方中断了, 或者对方缓冲区满了
            sendedNum = 0;
            if (errno != EWOULDBLOCK) {
                cout << "TcpConnection write error" << endl;
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    // 可能没有使能, 可能缓冲区还有数据, 可能没发完, 可能发完了所有
    assert(remaining <= len);
    // 先排除错误情况, 剩下的如果超过高水位, 那么就将高水位cb放入queue中
    if (!faultError && remaining > 0) {
        size_t oldLen = m_outputBuffer.readableBytesNum();
        if (oldLen + remaining >= m_highWaterMark &&
            oldLen < m_highWaterMark &&
            m_highWaterMarkCallback) {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, std::make_shared<TcpConnection>(*this), oldLen + remaining));
        }

        m_outputBuffer.append(static_cast<const char*>(msg + sendedNum), remaining); // 移动指针到剩余位置.
        if (!m_channel->isWriting()) {
            m_channel->setWriteEnable();
        }
    }
}
void TcpConnection::sendInLoop(const StringPiece& msg) {
    sendInLoop(msg.data(), msg.length());
}
void TcpConnection::send(const StringPiece& strMsg) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(strMsg.data(), strMsg.length());
        }
        else {
            void (TcpConnection:: * fp)(const StringPiece&) = &TcpConnection::sendInLoop;
            m_loop->runInLoop(std::bind(fp, this, strMsg));
        }
    }
}
void TcpConnection::send(const std::string& message, size_t len) {
    send(StringPiece(static_cast<const char*>(message.data()), len));
}
void TcpConnection::send(Buffer* buf) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytesNum());
            buf->retrieveAll();
        }
        else {
            void (TcpConnection:: * fp)(const StringPiece&) = &TcpConnection::sendInLoop;
            m_loop->runInLoop(std::bind(fp, this, buf->retrieveAllAsString())); // 把buffer的全retrieve, 然后送出来的string放到sendInLoop中.
        }
    }
}

// 与关闭嘻嘻相关
void TcpConnection::shutdown() { // 从已连接的状态到正在关闭的状态, 然后调用shutdownInLoop
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}
void TcpConnection::shutdownInLoop() {
    if (!m_channel->isWriting()) {
        m_socket->shutdownWrite();
    }
}
void TcpConnection::forceClose() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, this));
    }
}
void TcpConnection::forceCloseWithDelay(double seconds) {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->runAfter(seconds, std::bind(&TcpConnection::forceClose, this));
    }
}
void TcpConnection::forceCloseInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected || m_state == kDisconnecting) {
        handleClose(); // 直接触发关闭连接
    }
}
void TcpConnection::handleClose() {
    m_loop->assertInLoopThread();
    if (m_state == kDisconnected) {
        return;
    }
    assert(m_state == kConnected || m_state == kDisconnecting);
    setState(kDisconnected);
    m_channel->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    m_connectionCallback(guardThis); // 直接调用本地存储的连接回调函数
    m_closeCallback(guardThis); // 本地存储的关闭回调函数
}



// 设置
void TcpConnection::setTcpNoDelay(bool on) {
    m_socket->setTcpNoDelay(on);
}


// 读相关
void TcpConnection::startRead() { // 开始读的意思其实就是将channel设置为读感兴趣.
    m_loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}


/**
 * @brief 在tcpConnection中设置ET模式.
 *
 */
void TcpConnection::startReadInLoop() {
    m_loop->assertInLoopThread();
    if (!isreading || !m_channel->isReading()) {
        m_channel->setET();
        m_channel->setReadEnable();
        isreading = true;
    }
}
void TcpConnection::stopRead() {
    m_loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}
void TcpConnection::stopReadInLoop() {
    m_loop->assertInLoopThread();
    if (isreading || m_channel->isReading()) {
        m_channel->setReadDisable();
        isreading = false;
    }
}



// 从正在连接到连接成功的转换, 使能读, 然后执行连接成功的回调函数
void TcpConnection::connectEstablished() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    // auto mm = std::make_shared<TcpConnection>(*this);
    auto cc = shared_from_this();
    m_channel->settie(cc);
    m_channel->setReadEnable();  // tcpConnection专门用来对接客户端的channel.
    // 这里channel要联系上这个实例， 所以这里导致出错，到时候改一下
    // auto mm = std::make_shared<TcpConnection>(*this);
    // auto aa = mm.use_count();
    m_connectionCallback(cc);
    // int a = 1;
}


// 断开连接, handleClose最终也是调用这个
void TcpConnection::connectDestroyed() {
    m_loop->assertInLoopThread();
    timerid_ = TimerId(nullptr, -1); // 连接断开, 重置timerid
    if (m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();
        m_connectionCallback(shared_from_this());
        close(m_channel->showfd());
    }
    if (m_channel->showidx() != kNew) { // 这里必须要这样不然重复关闭
        m_channel->remove();
    }
}

// 读句柄, 当需要读的时候调用这个, 会读数据到本buffer中, 当触发可读时, 会调用这个句柄.
void TcpConnection::handleRead(Timestamp receiveTime) {
    // auto iiii = shared_from_this().use_count();
    // cout << iiii << endl;
    m_loop->assertInLoopThread();
    int savedErrno = 0;
    // 为了不出现错误, 这里清空inputbuf
    m_inputBuffer.retrieveAll();
    long long n = m_inputBuffer.readFd(m_socket->fd(), &savedErrno);
    // iiii = shared_from_this().use_count();./
    // cout << "n = " << n << endl;
    if (n > 0) {
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    }
    else if (n == 0) { // 对端关闭了
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
        }
        else {
            // 对端关闭
            handleClose();
        }
    }
    else { // 出错了
        errno = savedErrno;
        connectDestroyed();
    }
}

// 写句柄, 调用这个句柄将缓冲区的写出去, 当然前提是channel使能写了.  ET模式写入
void TcpConnection::handleWrite() {
    m_loop->assertInLoopThread();
    if (m_channel->isWriting()) {
        while (1) {
            ssize_t n = write(m_channel->showfd(), m_outputBuffer.peek(), m_outputBuffer.readableBytesNum());
            if (n > 0) {
                m_outputBuffer.retrieve(n);

            }
            else if (n == 0) {
                // 输出写外了, 
                if (m_outputBuffer.readableBytesNum() == 0) {
                    m_channel->setWriteDisable();
                    if (m_writeCompleteCallback) {
                        m_loop->queueInLoop(std::bind(m_writeCompleteCallback, std::make_shared<TcpConnection>(*this)));
                    }
                    if (m_state == kDisconnecting) {
                        shutdownInLoop();
                    }
                    break;  // 写外了就退出循环, 不然下一轮再进入循环由于我们已经shutdown了写, 会导致write那里出错
                }
            }
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 输出缓冲区满了, 暂时不处理, 等下次写, 返回epoll处, 等到输出缓冲区为空的时候(ET模式), 会再次触发写事件
                    break;
                }
                else {
                    LOG_ERROR << "TcpConnection::handleWrite() error";
                }
            }
        }
    }
    else {
        LOG_ERROR << "TcpConnection::handleWrite() error";
    }
}
// void TcpConnection::handleWrite() {
//     m_loop->assertInLoopThread();
//     if (m_channel->isWriting()) {
//         ssize_t n = write(m_channel->showfd(), m_outputBuffer.peek(), m_outputBuffer.readableBytesNum());
//         if (n > 0) {
//             m_outputBuffer.retrieve(n);
//             if (m_outputBuffer.readableBytesNum() == 0) {
//                 m_channel->setWriteDisable();
//                 if (m_writeCompleteCallback) {
//                     m_loop->queueInLoop(std::bind(m_writeCompleteCallback, std::make_shared<TcpConnection>(*this)));
//                 }
//                 if (m_state == kDisconnecting) {
//                     shutdownInLoop();
//                 }
//             }
//         }
//         else {
//             std::cout << "TcpConnection::handleWrite() no write any" << std::endl;
//         }
//     }
//     else {
//         std::cout << "TcpConnection::handleWrite() error" << std::endl;
//     }
// }




// 出错句柄
void TcpConnection::handleError() {
    m_loop->assertInLoopThread();
    // std::cout << "TcpConnection::handleError() " << std::endl;
    // handleClose();
}

