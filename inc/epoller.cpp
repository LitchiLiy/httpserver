#include <epoller.h>
#include <channel.h>
#include <eventLoop.h>
#include <sys/epoll.h>
#include <string.h>



const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;


Epoller::Epoller(EventLoop* el) : m_El(el) {
    // epoll初始化
    m_Epollfd = epoll_create1(EPOLL_CLOEXEC);
    cout << "mEpollfd: " << m_Epollfd << endl;
    m_reventVec = vector<struct epoll_event>(16);
}

Epoller::~Epoller() {

}
/*
    @brief: 他只对int fd, int event, revent和map进行改动.
*/
void Epoller::updateChannel(Channel* ch) {
    // 对于新的或者是已经删除的， 就新添加， 不然就修改或删除
    // 先判断idx是否有值.
    int ch_idx = ch->showidx();
    if (ch_idx == kNew || ch_idx == kDeleted) {

        if (ch_idx == kNew) {
            assert(m_channels.find(ch->showfd()) == m_channels.end());
            m_channels[ch->showfd()] = ch;
        }
        else {
            // kDeleted
            assert(m_channels.find(ch->showfd()) != m_channels.end());
            assert(m_channels[ch->showfd()] == ch); // mchannel, 移除频道和删除channel是两码事
        }
        ch->set_index(kAdded);
        epollupdate(EPOLL_CTL_ADD, ch);
    }
    else {
        int fdch = ch->showfd();
        assert(m_channels.find(fdch) != m_channels.end());
        assert(m_channels[fdch] == ch);
        assert(ch->showidx() == kAdded);
        if (ch->isNoneEvent()) {
            // 不对任何感兴趣
            epollupdate(EPOLL_CTL_DEL, ch);
            ch->set_index(kDeleted);
        }
        else {
            epollupdate(EPOLL_CTL_MOD, ch);
        }
    }
}

void Epoller::epollupdate(int op, Channel* ch) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = ch->showEvent();
    ev.data.ptr = ch;
    ev.data.fd = ch->showfd();
    if (epoll_ctl(m_Epollfd, op, ch->showfd(), &ev) < 0) {
        perror("epoll_ctl error");
    }
}

void Epoller::removeChannel(Channel* ch) {
    int chfd = ch->showfd();
    int chidx = ch->showidx();
    assert(m_channels.find(chfd) != m_channels.end());
    assert(m_channels[chfd] == ch);
    assert(ch->isNoneEvent());
    assert(chidx == kAdded || chidx == kDeleted);
    m_channels.erase(chfd);
    if (chidx == kAdded) {  // 从epoll中移除
        epollupdate(EPOLL_CTL_DEL, ch);
    }
    ch->set_index(kNew);
}

void Epoller::fillActChannel(int actNum, vector<Channel*>* actchannels) {
    // 就是把相应的channal指针放到act里面去

    for (int it = 0; it < actNum; it++) {
        // 找到对应的channal
        auto mapit = m_channels.find(m_reventVec[it].data.fd);
        if (mapit != m_channels.end()) {
            // 两个任务:channal设置revent, 推入act中
            mapit->second->set_rEvent(m_reventVec[it].events);  // channel发生的事件写入channel中
            actchannels->push_back(mapit->second);  // 这个事件写入vec
        }
    }
}

Timestamp Epoller::epolling(int timeoutMs, vector<Channel*>* actchannels) {

    int actNum = 0;
    actNum = epoll_wait(m_Epollfd, &*m_reventVec.begin(), m_reventVec.size(), timeoutMs);
    int saveErrno = errno; // epollwait之后

    Timestamp now = Timestamp::now();
    if (actNum > 0) {
        fillActChannel(actNum, actchannels);
        if (static_cast<size_t>(actNum) == m_reventVec.size()) // 数组不够大再扩容
        {
            m_reventVec.resize(m_reventVec.size() * 2);
        }
    }
    else if (actNum == 0) {
        cout << "epoll_wait timeout, nothing happen" << endl;
    }
    else {
        cout << "errno = " << saveErrno << endl; // errno = 4就是收到了中断信号EINTR
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            cout << "EPollPoller::poll()" << endl;
        }
    }
    return now;
}

bool Epoller::hasChannel(Channel* ch) {
    m_El->assertInLoopThread();
    auto it = m_channels.find(ch->showfd());
    return it != m_channels.end() && it->second == ch;
}