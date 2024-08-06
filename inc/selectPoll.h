#if !defined(SELECTPOLL_H)
#define SELECTPOLL_H


#include <pollbase.h>
#include <sys/select.h>



/**
 * @brief selectIO复用模式
 *
 */
class SelectPoll : public Pollbase {
public:
    SelectPoll(EventLoop* el);
    ~SelectPoll() override;

    void updateChannel(Channel* Channel) override;
    Timestamp poll(int timeoutMs, ChannelList* actchannels);
    void fillActChannel(int actNum, ChannelList* actchannels);
    void removeChannel(Channel* channel);


    // 
private:

    fd_set readFds_;
    fd_set writeFds_;
    fd_set errorFds_;
};


#endif // SELECTPOLL_H
