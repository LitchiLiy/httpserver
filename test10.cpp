/*
测试Connector, 感受一下他是否可以循环的连接, 这个Connector类是用在客户端上的.


void Connector::retry(int sockfd) {
    close(sockfd);
    setState(kDisconnected);
    if (isconnected) {
        m_loop->runAfter(m_retryDelayMs / 1000.0,
            std::bind(&Connector::startInLoop, this));   // 毫秒值,
        m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
        cout << "在 " << m_retryDelayMs << "ms之后重试" << endl;
    }
    else {
        cout << "connector retry error" << endl;
    }
}
上述代码中, 原始版本的this位置是make_shared<Connector>(*this), 这会导致一个问题就是, 这个句子会根据this这个实例重新创建一个新实例, 导致所有内部成员变量全部初始化, 所以不要乱用makeshared来复制指针.

在C++中，当你使用 std::make_shared<Connector>(*this) 这样的表达式时，你实际上是在请求 std::make_shared 函数创建一个新的 Connector 对象，并且使用当前对象 *this 的值来初始化这个新对象。

如果你的构造函数没有复制构造, 那么就只能初始化构造了.
*/


#include <connector.h>
#include <eventLoop.h>
#include <mInetAddress.h>
#include <callBacks.h>
#include <stdio.h>


EventLoop* g_loop;

void connectCallback(int sockfd) {
    printf("Connected! \n");
    g_loop->quit();
};

int main() {
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 8888);

    ConnectorPtr connector(new Connector(&loop, addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->startCntor();


    loop.loop();
    return 0;


}


