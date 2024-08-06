#if !defined(CHANNEL_H)
#define CHANNEL_H



#include <functional>
#include <callBacks.h>
#include <sys/epoll.h>



class EventLoop;  // 只是作为指针


class Channel {
public:
    typedef std::function<void()> callBack_f;
    Channel(EventLoop* el_, int fd_);
    Channel() = default;
    ~Channel();

    // 设置回调
    void setReadCallBack(ReadEventcb_f cb);
    void setWriteCallBack(callBack_f cb);
    void setErroCallBack(callBack_f cb);
    void setCloseCallBakc(callBack_f cb) {
        m_CloseCb = std::move(cb);
    }

    // 使能
    void setReadEnable();
    void setWriteEnable();

    // 关闭使能
    void setReadDisable();
    void setWriteDisable();

    // 判断是否使能
    bool isWriting() const { return m_event & kWriteEvent; };
    bool isReading() const { return m_event & kReadEvent; };

    // 这三个表明的是该channel对什么感兴趣.
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    void handleEvent(Timestamp receiveTime);

    void set_index(int idx) { m_idx = idx; };
    int showidx() { return m_idx; };
    int showEvent() { return m_event; };
    void set_rEvent(int et) { m_revent = et; };
    int showfd() { return m_fd; };
    bool isNoneEvent() { return m_event == kNoneEvent; };
    EventLoop* ownerloop() { return m_El; };
    void disableAll() { m_event = kNoneEvent;  updateChannel(); }; // 更新自己的状态为无事件, 并对poller更新, 并对注册表进行修改
    bool showisaddedToLoop() { return isaddedToLoop; };
    void remove(); // 让poller删除自己, 并对注册表进行修改, 还把map中的也删掉了, 也就是彻底删除, 

    void settie(const std::shared_ptr<void>& p) {
        tie_ = p;
        istie = true;
    }

    // ET模式
    void setET() { m_event |= EPOLLET; };
private:
    void updateChannel();


private:
    EventLoop* m_El;
    int m_fd;

    ReadEventcb_f m_ReadCb;
    EventCb_f m_WriteCb;
    EventCb_f m_ErroCb;
    EventCb_f m_CloseCb;

    int m_event; // 用户想要感兴趣的情况
    int m_revent;

    // poller专用
    int m_idx = -1;
    bool isaddedToLoop = false;

    // tcpconnection
    bool iseventHandling = false;

    // 构建联系， 某个实例与Channel有关，这里将存放他的指针， 但是channel不会增加他的引用(用在handleEvent中)
    std::weak_ptr<void> tie_;
    bool istie; // 标记是否是tie的


};

#endif // CHANNEL_H
