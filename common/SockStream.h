#ifndef _SOCKSTREAM_H_
#define _SOCKSTREAM_H_

#include "Common.h"
#include "Error.h"

class SockStream
{
public:
    SockStream(){}
    SockStream(int fd)
    {
        this->fd = fd;
    }
    void init(int fd)
    {
        this->fd = fd;
    }
    
    /* Read "n" bytes from a fd. */
    ssize_t readn(void * vptr, size_t n);
    
    /* Wrapper for readn */
    ssize_t Readn(void * ptr, size_t nbytes);

    /* Wrapper for writen */
    void Writen(void * ptr, size_t nbytes);

    /* Write "n" bytes to a descriptor */
    ssize_t writen(const void * vptr, size_t n);

    /* bytes remained in read buffer. */
    ssize_t readlineBuf(void ** vptrptr);

    /* Wrapper for readline. */
    ssize_t Readline(void * ptr, size_t maxlen);

private:
    int fd;
};


#endif