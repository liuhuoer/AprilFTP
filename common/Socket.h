#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "Common.h"

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
private:
    SockType            socktype;
    const char *        host;
    short               port;
    int                 sockfd;
};


#endif