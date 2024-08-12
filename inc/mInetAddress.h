#if !defined(INETADDRESS_H)
#define INETADDRESS_H

/*
功能
1. 记录两个地址, 一个是ipv4一个是ipv6
2. 存入地址函数, 设置地址函数,
3. 构造函数初始化
4. 从ntop, 从pton, ntop就是给你一个sockaddr, 返回一个string, p
*/
#include <netinet/in.h>
#include <string>


using namespace std;



class InetAddress {
public:
    InetAddress(const std::string ip, uint16_t port, bool ipv6 = false);
    InetAddress() = default;
    explicit InetAddress(const struct sockaddr_in& addr)
        : m_addr(addr) {}
    explicit InetAddress(const struct sockaddr_in6& addr)
        : m_addr6(addr) {}
    ~InetAddress();

    void setSockAddr(const struct sockaddr_in& addr) { m_addr = addr; }
    void setSockAddr6(const struct sockaddr_in6& addr) { m_addr6 = addr; }

    const struct sockaddr_in& getSockAddr() const { return m_addr; }
    const struct sockaddr_in6& getSockAddr6() const { return m_addr6; }

    short ptonIp() const;
    in_addr_t ptonIpPort() const;


    /**
     * @brief 将ip地址字符串化
     *
     * @return string
     */
    string ipToString() const;

    static sockaddr_in getLocalAddr(int sockfd);
    static sockaddr_in getPeerAddr(int sockfd);



private:
    struct sockaddr_in m_addr;
    struct sockaddr_in6 m_addr6;
};

#endif // INETADDRESS_H
