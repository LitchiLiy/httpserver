#include <channel.h>
#include <sys/epoll.h>
#include <eventLoop.h>
#include <iostream>
#include <assert.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EventLoop* el_, int fd_) :m_El(el_), m_fd(fd_), m_event(0), m_idx(-1), m_revent(0) {

}

Channel::~Channel() {
    assert(!iseventHandling); // 是否正在处理
    assert(!isaddedToLoop); // 是否已经从任何Loop中移除
    if (m_El->isInLoopThread()) {
        assert(!m_El->hasChannel(this));  // 再次确认是否已经从该Loop中移除
    }
};

void Channel::setReadCallBack(ReadEventcb_f cb) {
    m_ReadCb = cb;
}
void Channel::setWriteCallBack(callBack_f cb) {
    m_WriteCb = cb;
}
void Channel::setErroCallBack(callBack_f cb) {
    m_ErroCb = cb;
}


void Channel::setReadEnable() {

    m_event |= kReadEvent;
    updateChannel();
}
void Channel::setWriteEnable() {
    m_event |= kWriteEvent;
    updateChannel();
}

void Channel::updateChannel() {
    isaddedToLoop = true;
    m_El->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
    iseventHandling = true;
    // 关于关闭Channel的情况
    if ((m_revent & EPOLLHUP) && !(m_revent & EPOLLIN)) {  // 对端关闭连接EPOLLHUP.
        std::cout << "Channel::handle_event() POLLHUP" << std::endl;
        if (m_CloseCb) {
            m_CloseCb();
        }
    }
    // 判断revent是什么类型执行什么回调函数.
    if (m_revent & (EPOLLERR)) {
        if (m_ErroCb) {
            m_ErroCb();
        }
    }

    if (m_revent & (EPOLLIN | EPOLLPRI | EPOLLHUP)) {
        if (m_ReadCb) {
            m_ReadCb(receiveTime);
        }
    }

    if (m_revent & EPOLLOUT) {
        if (m_WriteCb) {
            m_WriteCb();
        }
    }
    iseventHandling = false;
}

void Channel::remove() {
    assert(isNoneEvent());
    isaddedToLoop = false;
    m_El->remove(this);
}

void Channel::setReadDisable() {
    m_event &= ~kReadEvent;
    updateChannel();
}
void Channel::setWriteDisable() {
    m_event &= ~kWriteEvent;
    updateChannel();
}