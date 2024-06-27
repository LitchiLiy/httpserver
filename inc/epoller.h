#if !defined(EPOLLER_H) 
#define EPOLLER_H


#include <sys/epoll.h>
#include <vector>
#include <unordered_map>



class Channel;
using namespace std;
class EventLoop;

class Timestamp;
class Epoller {

public:
    Epoller(EventLoop* el);
    ~Epoller();

    void updateChannel(Channel* ch);
    Timestamp epolling(int timeoutMs, vector<Channel*>* actchannels);
    void fillActChannel(int actNum, vector<Channel*>* actchannels);
    void removeChannel(Channel* ch);

    // 确认是否移除channel
    bool hasChannel(Channel* ch);


private:
    void epollupdate(int op, Channel* ch);


private:
    EventLoop* m_El;
    int m_Epollfd;
    vector<struct epoll_event> m_reventVec; // 接收epoll结果
    unordered_map<int, Channel*> m_channels; // fd, channal队列
};



#endif // EPOLLER_H)    
