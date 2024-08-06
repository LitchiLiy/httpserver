#include <connector.h>


#include <eventLoop.h>
#include <mInetAddress.h>
#include <channel.h>    
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>



using namespace std;


const int Connector::kMaxRetryDelayMs;


// 比较对方的fd是不是和自己的fd是一样的
bool cmpPeerLocfd(int fd) {
    sockaddr_in locAddr = InetAddress::getLocalAddr(fd);
    sockaddr_in peerAddr = InetAddress::getPeerAddr(fd);

    return locAddr.sin_port == peerAddr.sin_port && locAddr.sin_addr.s_addr == peerAddr.sin_addr.s_addr;
}

Connector::Connector(EventLoop* lp, const InetAddress& serverAddr) :
    m_loop(lp),
    m_ServerAddr(serverAddr),
    m_state(kDisconnected),
    isconnected(false),
    m_retryDelayMs(Connector::kInitRetryDelayMs)
{
}
Connector::~Connector()
{
    assert(!m_channel);
}

void Connector::startCntor()
{
    // 任何人调用这个函数, 都将跑到IO线程中去实现.
    isconnected = true; // 一开机就认定保持连接
    m_loop->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == kDisconnected);
    if (isconnected) {
        connect();
    }
    else {
        cout << "connector is not connected" << endl;
    }
}


void Connector::stopCntor() {
    isconnected = false;
    m_loop->queueInLoop(std::bind(&Connector::stopInLoop, this));
}
void Connector::stopInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == kConnecting) { // 正在连接的时候, 如果触发了停止, 进行操作
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    // 核心函数, 创建一个fd根据服务器地址, 然后连接服务器, 然后对错误进行判断
    // 这里只对ipv4能成
    int sockfd = socket(m_ServerAddr.getSockAddr().sin_family, SOCK_STREAM, 0);
    // 非阻塞, exec
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SOCK_NONBLOCK | SOCK_CLOEXEC, &opt, sizeof(opt));

    int ret = ::connect(sockfd, (struct sockaddr*)&m_ServerAddr.getSockAddr(), sizeof(m_ServerAddr.getSockAddr()));

    int saveErrno = (ret == 0) ? 0 : errno;

    switch (saveErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        break;
        connecting(sockfd); // 如果fd创建成功并且正在连接的时候, 进行连接
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        // 连接失败, 重试
        cout << "connect error in Connector::startInLoop, Retry delay" << saveErrno << endl;
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        std::cout << "Unexpected error in Connector::startInLoop, close FD " << saveErrno << endl;
        close(sockfd);
        break;
    }
}

// 重启就是重新走一遍Connect流程. 
void Connector::restartCntor() {
    m_loop->assertInLoopThread();
    setState(kDisconnected);
    m_retryDelayMs = Connector::kInitRetryDelayMs;
    isconnected = true;
    startInLoop();
}


void Connector::connecting(int sockfd) {
    setState(kConnecting);
    assert(!m_channel);

    m_channel.reset(new Channel(m_loop, sockfd)); // sharedptr的reset, 用来更换channel

    m_channel->setWriteCallBack(std::bind(&Connector::handleWrite, this));
    m_channel->setErroCallBack(std::bind(&Connector::handleError, this));

    m_channel->setWriteEnable(); // 新建立的channel使能往里面写.

}

// 移除channel的任何地方的存在, 然后将这个channel指向空, 返回这个channel的fd
int Connector::removeAndResetChannel() {
    m_channel->disableAll(); // 禁用所有的事件,并update
    m_channel->remove(); // 移除channel, 从注册上移除
    int sockfd = m_channel->showfd();
    // m_channel.reset(); 直接让channel指向空
    m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel() {
    m_channel.reset(); // 指向空
}


// 执行写操作, 
void Connector::handleWrite() {
    if (m_state == kConnecting) {
        int sockfd = removeAndResetChannel();
        int err;
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            err = errno;
        }
        else
        {
            err = optval;
        }

        if (err) {
            cout << "connector error" << err << endl;
            retry(sockfd); // 有错误就重试
        }
        else if (cmpPeerLocfd(sockfd)) {
            cout << "connect oneself" << endl;
            retry(sockfd);
        }
        else {
            setState(kConnected);
            if (isconnected) {
                m_newConnectionCallback(sockfd); // 此时的状态是, fd已连接, 但channel已经清空并置为空. 直接执行回调函数.
            }
            else {
                close(sockfd);
            }
        }
    }
    else {
        // 发生啥了? 
        assert(m_state == kDisconnected);
    }

}

void Connector::handleError() {
    cout << "connector handleError" << endl;
    if (m_state == kConnecting) {
        int sockfd = removeAndResetChannel(); // 先移除Channel的所有存在
        int err;
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            err = errno;
        }
        else
        {
            err = optval;
        }
        cout << "Connector::handleError - SO_ERROR = " << err << endl;
        retry(sockfd); // 有错误就重试
    }
}

void Connector::retry(int sockfd) {
    close(sockfd);
    setState(kDisconnected);
    if (isconnected) {
        m_loop->runAfter(m_retryDelayMs / 1000.0,
            std::bind(&Connector::startInLoop, this));   // 毫秒值, 
        m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
        // cout << "在 " << m_retryDelayMs << "ms之后重试" << endl;
    }
    else {
        cout << "connector retry error" << endl;
    }
}