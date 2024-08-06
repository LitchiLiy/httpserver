#if !defined(EPOLLER_H) 
#define EPOLLER_H


#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <pollbase.h>



class Channel;
using namespace std;
class EventLoop;
class Timestamp;

class Epoller : public Pollbase {

public:
    Epoller(EventLoop* el);
    ~Epoller() override;

    void updateChannel(Channel* Channel) override;
    Timestamp poll(int timeoutMs, ChannelList* actchannels) override;
    void fillActChannel(int actNum, ChannelList* actchannels) override;
    void removeChannel(Channel* channel) override;

    // 确认是否移除channel
    int getEpollfd() { return m_Epollfd; }


private:
    void epollupdate(int op, Channel* ch);


private:
    int m_Epollfd;
    vector<struct epoll_event> m_reventVec; // 接收epoll结果
    // unordered_map<int, Channel*> m_channels; // fd, channal队列
};



#endif // EPOLLER_H)    
