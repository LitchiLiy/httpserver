#if !defined(CALLBACKS_H)
#define CALLBACKS_H

#include <functional>
#include <memory>

class TcpConnection;
class EventLoop;
class Timestamp;
class Connector;


typedef std::function<void()> callBack_f;
typedef std::function<void(EventLoop*)> threadInitCallBack;

// 关于TcpConnection
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(TcpConnectionPtr, Buffer*, Timestamp)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

// channel
class Timestamp;
typedef std::function<void()> EventCb_f;
typedef std::function<void(Timestamp)> ReadEventcb_f;

// Connector
typedef std::function<void(int sockfd)> NewConnectionCallback;
typedef std::shared_ptr<Connector> ConnectorPtr;



#endif // CALLBACKS_H
