#include <mInetAddress.h>
#include <string.h>
#include <arpa/inet.h>


InetAddress::InetAddress(const std::string ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        memset(&m_addr6, 0, sizeof(m_addr6));
        m_addr6.sin6_family = AF_INET6;
        m_addr6.sin6_port = htons(port);
        inet_pton(AF_INET6, ip.c_str(), &m_addr6.sin6_addr);
    }
    else {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
    }
}


InetAddress::~InetAddress() {}

short InetAddress::ptonIp() const {
    return ntohs(m_addr.sin_port);
}


in_addr_t InetAddress::ptonIpPort() const {
    return ntohl(m_addr.sin_addr.s_addr);
}


struct sockaddr_in InetAddress::getLocalAddr(int sockfd) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getpeername(sockfd, (sockaddr*)&addr, &len) < 0) {
        perror("getpeername");
    }
    return addr;
}


struct sockaddr_in InetAddress::getPeerAddr(int sockfd) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getpeername(sockfd, (sockaddr*)&addr, &len) < 0) {
        perror("getpeername");
    }
    return addr;
}


string InetAddress::ipToString() const {
    char buf[INET6_ADDRSTRLEN] = { 0 };
    if (m_addr.sin_family == AF_INET) {
        inet_ntop(AF_INET, &m_addr.sin_addr, buf, INET_ADDRSTRLEN);
    }
    return string(buf) + ":" + to_string(ntohs(m_addr.sin_port));
}