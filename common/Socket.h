#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "Common.h"
#include "Error.h"

class Socket
{
public:
    Socket(SockType socktype,
            const char *host,
            short port)
    {
        this->socktype = socktype;
        this->host = host;
        this->port = port;
    }
    int                 init();
    static int          tcpAccept(int fd, sockaddr *sa, socklen_t *salenptr);
    void                tcpBind(int fd, const sockaddr *sa, socklen_t salen);
    void                tcpConnect(int fd, const sockaddr *sa, socklen_t salen);
    void                tcpListen(int fd, int backlog);
    ssize_t             tcpRecv(int fd, void *ptr, size_t nbytes, int flags);
    void                tcpSend(int fd, const void *ptr, size_t nbytes, int flags);
    void                tcpSetsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
    void                tcpShutdown(int fd, int how);
    int                 tcpSocket(int family, int type, int protocol);
    static void         tcpClose(int fd);

private:
    SockType            socktype;
    const char *        host;
    short               port;
    int                 sockfd;
};


#endif