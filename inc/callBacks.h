#if !defined(CALLBACKS_H)
#define CALLBACKS_H

#include <functional>

class EventLoop;
typedef std::function<void()> callBack_f;
typedef std::function<void(EventLoop*)> threadInitCallBack;

#endif // CALLBACKS_H
