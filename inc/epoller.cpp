#include <epoller.h>
#include <channel.h>
#include <eventLoop.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>






Epoller::Epoller(EventLoop* el) :Pollbase(el) {
    // epoll初始化
    m_Epollfd = epoll_create1(EPOLL_CLOEXEC);
    // cout << "mEpollfd: " << m_Epollfd << endl;
    m_reventVec = vector<struct epoll_event>(16);
}

Epoller::~Epoller() {
    close(m_Epollfd);
}
/*
    @brief: 他只对int fd, int event, revent和map进行改动.
*/
void Epoller::updateChannel(Channel* channel) {
    // 对于新的或者是已经删除的， 就新添加， 不然就修改或删除
    // 先判断idx是否有值.
    int ch_idx = channel->showidx();
    if (ch_idx == kNew || ch_idx == kDeleted) {

        if (ch_idx == kNew) {
            assert(m_channels.find(channel->showfd()) == m_channels.end());
            m_channels[channel->showfd()] = channel;
        }
        else {
            // kDeleted
            assert(m_channels.find(channel->showfd()) != m_channels.end());
            assert(m_channels[channel->showfd()] == channel); // mchannel, 移除频道和删除channel是两码事
        }
        channel->set_index(kAdded);
        epollupdate(EPOLL_CTL_ADD, channel);
    }
    else {
        int fdch = channel->showfd();
        assert(m_channels.find(fdch) != m_channels.end());
        assert(m_channels[fdch] == channel);
        assert(channel->showidx() == kAdded);
        if (channel->isNoneEvent()) {
            // 不对任何感兴趣
            epollupdate(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            epollupdate(EPOLL_CTL_MOD, channel);
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

void Epoller::removeChannel(Channel* channel) {
    int chfd = channel->showfd();
    int chidx = channel->showidx();
    assert(m_channels.find(chfd) != m_channels.end());
    assert(m_channels[chfd] == channel);
    assert(channel->isNoneEvent());
    assert(chidx == kAdded || chidx == kDeleted);
    m_channels.erase(chfd);
    if (chidx == kAdded) {  // 从epoll中移除
        epollupdate(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void Epoller::fillActChannel(int actNum, ChannelList* actchannels) {
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

Timestamp Epoller::poll(int timeoutMs, ChannelList* actchannels) {

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
        LOG_ERROR << "epoller::poll() timeout";
    }
    else {
        LOG_ERROR << "epoller::poll() error: " << strerror(saveErrno); // errno = 4就是收到了中断信号EINTR
        if (saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR << "epoller::poll() error: " << strerror(errno);
        }
    }
    return now;
}

