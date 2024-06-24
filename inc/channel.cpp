#include <channel.h>
#include <sys/epoll.h>
#include <eventLoop.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EventLoop* el_, int fd_) :m_El(el_), m_fd(fd_), m_event(0), m_idx(-1), m_revent(0) {

}

Channel::~Channel() {

};

void Channel::setReadCallBack(callBack_f cb) {
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

void Channel::handleEvent() {
    // 判断revent是什么类型执行什么回调函数.
    if (m_revent & (EPOLLERR)) {
        if (m_ErroCb) {
            m_ErroCb();
        }
    }

    if (m_revent & (EPOLLIN | EPOLLPRI | EPOLLHUP)) {
        if (m_ReadCb) {
            m_ReadCb();
        }
    }

    if (m_revent & EPOLLOUT) {
        if (m_WriteCb) {
            m_WriteCb();
        }
    }
}

void Channel::remove() {
    assert(isNoneEvent());
    isaddedToLoop = false;
    m_El->remove(this);
}




