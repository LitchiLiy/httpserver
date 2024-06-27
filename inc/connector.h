#if !defined(CONNECTOR_H)
#define CONNECTOR_H
#include <memory>


using namespace std;



#include <callBacks.h>
#include <mInetAddress.h>



class EventLoop;
class Channel;

class Connector {
public:
    Connector(EventLoop* lp, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { m_newConnectionCallback = cb; }

    void startCntor();
    void restartCntor();
    void stopCntor();

    const InetAddress& showServerAddr() const { return m_ServerAddr; }




private:
    enum States { kDisconnected, kConnecting, kConnected };

    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { m_state = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* m_loop;
    InetAddress m_ServerAddr;
    bool isconnected;
    States m_state;
    shared_ptr<Channel> m_channel;
    NewConnectionCallback m_newConnectionCallback;
    int m_retryDelayMs = 500; // 毫秒值
};


#endif // CONNECTOR_H)
