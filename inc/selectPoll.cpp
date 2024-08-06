#include <selectPoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <timestamp.h>
#include <channel.h>
#include <logging.h>
#include <errno.h>
using namespace std;
#include <iostream>



SelectPoll::SelectPoll(EventLoop* el) : Pollbase(el) {
    // 初始化readFds_和writeFds_
    FD_ZERO(&readFds_);
    FD_ZERO(&writeFds_);
    FD_ZERO(&errorFds_);
}
SelectPoll::~SelectPoll() {

}

Timestamp SelectPoll::poll(int timeoutMs, ChannelList* actchannels) {
    // 初始化readFds_和writeFds_
    FD_ZERO(&readfd_tmp);
    FD_ZERO(&writefd_tmp);
    FD_ZERO(&errorfd_tmp);

    readfd_tmp = readFds_;
    writefd_tmp = writeFds_;
    errorfd_tmp = errorFds_;

    // listen数量默认为10
    int numEvents = ::select(FD_SETSIZE, &readfd_tmp, &writefd_tmp, &errorfd_tmp, nullptr); // 这里如果不是fdsetsize, 则没法识别到文件, fdsetsize为1024 nullptr表示无限期阻塞
    Timestamp now(Timestamp::now());
    // cout << "1" << endl;
    int saveErrno = errno;

    if (numEvents > 0) {
        fillActChannel(numEvents, actchannels);
    }
    else if (numEvents == 0) {
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


void SelectPoll::fillActChannel(int numEvents, ChannelList* actchannels) {
    for (int i = 0; i < numEvents; ++i) {
        // 循环本地mchannel, 找到响应的fd
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            Channel* channel = it->second;
            int fd = channel->showfd();
            if (channel->isReading() && FD_ISSET(fd, &readfd_tmp)) {

                channel->set_rEvent(EPOLLIN);
                actchannels->push_back(channel);
            }
            if (channel->isWriting() && FD_ISSET(fd, &writefd_tmp)) {
                channel->set_rEvent(EPOLLOUT);
                actchannels->push_back(channel);
            }
        }
    }
}

void SelectPoll::removeChannel(Channel* channel) {
    int chfd = channel->showfd();
    int chidx = channel->showidx();
    assert(m_channels.find(chfd) != m_channels.end());
    assert(m_channels[chfd] == channel);
    assert(channel->isNoneEvent());


    assert(chidx == kAdded || chidx == kDeleted);
    m_channels.erase(chfd);


    // if (chidx == kAdded) {  // 从epoll中移除
    //     epollupdate(EPOLL_CTL_DEL, ch);
    // }
    // select只需要在对应set里删除就行了
    if (chidx == kAdded) {
        if (channel->isReading() && FD_ISSET(chfd, &readFds_)) {
            FD_CLR(chfd, &readFds_);
        }
        if (channel->isWriting() && FD_ISSET(chfd, &writeFds_)) {
            FD_CLR(chfd, &writeFds_);
        }
    }
    channel->set_index(kNew);
}

void SelectPoll::updateChannel(Channel* channel) {
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
        if (channel->isReading()) {
            FD_SET(channel->showfd(), &readFds_);
        }
        if (channel->isWriting()) {
            FD_SET(channel->showfd(), &writeFds_);
        }
    }
    else {
        int fdch = channel->showfd();
        assert(m_channels.find(fdch) != m_channels.end());
        assert(m_channels[fdch] == channel);
        assert(channel->showidx() == kAdded);
        if (channel->isNoneEvent()) {
            // 不对任何感兴趣
            if (channel->isReading() && FD_ISSET(fdch, &readFds_)) {
                FD_CLR(fdch, &readFds_);
            }
            if (channel->isWriting() && FD_ISSET(fdch, &writeFds_)) {
                FD_CLR(fdch, &writeFds_);
            }
            channel->set_index(kDeleted);
        }
        else {
            // 修改
            // 先删除

            if (FD_ISSET(fdch, &readFds_)) {
                FD_CLR(fdch, &readFds_);
            }
            if (FD_ISSET(fdch, &writeFds_)) {
                FD_CLR(fdch, &writeFds_);
            }
            // 再添加
            if (channel->isReading() && !FD_ISSET(fdch, &readFds_)) {
                FD_SET(fdch, &readFds_);
            }
            if (channel->isWriting() && !FD_ISSET(fdch, &writeFds_)) {
                FD_SET(fdch, &writeFds_);
            }
        }
    }
}

