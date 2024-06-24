#if !defined(CHANNEL_H)
#define CHANNEL_H



#include <functional>


class EventLoop;  // 只是作为指针


class Channel {
public:
    typedef std::function<void()> callBack_f;
    Channel(EventLoop* el_, int fd_);
    Channel() = default;
    ~Channel();

    // 设置回调
    void setReadCallBack(callBack_f cb);
    void setWriteCallBack(callBack_f cb);
    void setErroCallBack(callBack_f cb);

    // 使能
    void setReadEnable();
    void setWriteEnable();


    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    void handleEvent();

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



private:
    void updateChannel();


private:
    EventLoop* m_El;
    int m_fd;

    callBack_f m_ReadCb;
    callBack_f m_WriteCb;
    callBack_f m_ErroCb;

    int m_event; // 用户想要感兴趣的情况
    int m_revent;

    // poller专用
    int m_idx = -1;
    bool isaddedToLoop = false;

};

#endif // CHANNEL_H
