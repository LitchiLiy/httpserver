#if !defined(POLLFACTORY_H)
#define POLLFACTORY_H

#include <selectPoll.h>
#include <epoller.h>
#include <logging.h>

/**
 * @brief 一个简单工厂
 *
 */
class EventLoop;

class PollFactory {
public:
    PollFactory() {};
    Pollbase* createPoll(const string& str, EventLoop* elp) {
        if (str == "epoll") {
            return new Epoller(elp);
        }
        else if (str == "select") {
            return new SelectPoll(elp);
        }
        else {
            LOG_ERROR << "pollmode is not right, please check it";
        }
    }
};



#endif // POLLFACTORY_H
