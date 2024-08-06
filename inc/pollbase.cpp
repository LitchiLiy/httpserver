#include <pollbase.h>
#include <eventLoop.h>
#include <channel.h>
#include <eventLoop.h>
#include <string>
#include <string.h>
#include <timestamp.h>
#include <logging.h>

string pollmode = "";


Pollbase::Pollbase(EventLoop* loop)
    : m_El(loop) {}

Pollbase::~Pollbase() = default;

/**
 * @brief 寻找一个channel是否在本poll中存储, 或者说检查我们是否有监视某个channel
 *
 * @param channel
 * @return true
 * @return false
 */
bool Pollbase::findChannel(Channel* channel) const {
    m_El->assertInLoopThread();
    ChannelMap::const_iterator it = m_channels.find(channel->showfd());
    return it != m_channels.end() && it->second == channel;
}
