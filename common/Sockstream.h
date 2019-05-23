#ifndef _SOCKSTREAM_H_
#define _SOCKSTREAM_H_

#include "Common.h"
#include "Error.h"

class Sockstream
{
public:
    SockStream(){};
    SockStream(int fd)
    {
        this->fd = fd;
    }
    void init(int fd)
    {
        this->fd = fd;
    }
};

#endif