#if !defined(POLLBASE_H)
#define POLLBASE_H



#include <vector>
#include <unordered_map>
#include <string>



class Channel;

class EventLoop;

class Timestamp;

using namespace std;
/**
 * @brief 简单提供了channel管理的结构, 还有poll的功能, 还有channel的记录和evetnloop的记录
 *
 */
class Pollbase {
public:
    typedef std::vector<Channel*> ChannelList;

    Pollbase(EventLoop* loop);
    virtual ~Pollbase();


    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual void fillActChannel(int actNum, ChannelList* actchannels) = 0;
    virtual bool findChannel(Channel* channel) const;


    string showMode() {
        return mode;
    }


protected:
    typedef std::unordered_map<int, Channel*> ChannelMap; // fd和channel的地址的map
    ChannelMap m_channels;
    EventLoop* m_El;
private:
    string mode;
};

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

#endif // POLLBASE_H
