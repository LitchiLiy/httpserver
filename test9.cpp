/*
目的建立一个EchoServer
*/



#include <eventLoop.h>
#include <mAcceptor.h>
#include <mInetAddress.h>
#include <tcpServer.h>
#include <iostream>
#include <string>
#include <callBacks.h>
#include <tcpConnection.h>
#include <unistd.h>



/*
测试TcpServer的功能, 本地服务器listen端口地址为127.0.0.1, 端口为8888, 调用test7_2.cpp的客户端来连接这个服务器, 然后服务器给客户端发送一个hello world以表示建立好了连接.

由于此时没有构建EventLoopTheadPool, 故暂时使用一个单独的EventLoop来完成所有操作.
*/






using namespace std;






class EchoServer {
public:
    EchoServer(EventLoop* el_, const InetAddress& listenAddr_) :
        m_loop(el_),
        m_server(el_, listenAddr_, "EchoServer")
    {
        m_server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        m_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_server.setThreadNum(1);
    }


    void start() {
        m_server.start();
    }


private:


    void onConnection(TcpConnectionPtr conn) {
        // 此时, loop将会再TcpConnection的Read Channel触发之后, 在loop中调用这个回调函数.
        cout << "客户端连接成功" << endl;
        conn->send("hello world\n");
    }


    void onMessage(TcpConnectionPtr conn, Buffer* buf, Timestamp time) {
        string msg(buf->retrieveAllAsString());

        if (msg == "exit\n") {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n") {
            m_loop->quit();
        }
        conn->send(msg);
    }

private:
    EventLoop* m_loop;
    TcpServer m_server;
};



int main() {
    EventLoop loop;
    cout << "当前线程的Pid为:" << getpid() << ", 当前线程的tid为:" << pthread_self() << endl;

    InetAddress listenAddr("127.0.0.1", 8888, false);

    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}
