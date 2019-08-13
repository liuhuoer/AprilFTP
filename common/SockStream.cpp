#include "SockStream.h"

/* Read "n" bytes from a fd. */
ssize_t SockStream::readn(void * vptr, size_t n)
{
    size_t nleft;           /* Read "n" bytes from a descriptor. */
    ssize_t nread;
    char * ptr;

    ptr = (char *)vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            if(errno == EINTR)
            {
                nread = 0;  /* and call read() again*/
            }
            else
            {
                return(-1);
            }
        }else if(nread == 0){
            break;          /* return >= 0*/
        }
        
        nleft -= nread;
        ptr += nread;
    }
    return(n - nleft);
}

/* Wrapper for readn */
//ssize_t SockStream::Readn(void * ptr, size_t nbytes)
//{}


/* Write "n" bytes to a descriptor */
ssize_t SockStream::writen(const void * vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char * ptr;

    ptr = (const char *)vptr;
    nleft = n;
    while(nleft > 0)
    {
        if( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return(-1);
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);

}
/* Wrapper for writen */
void SockStream::Writen(void * ptr, size_t nbytes)
{
    ssize_t n;
    if( (n = writen(ptr, nbytes)) < 0 || (size_t)n != nbytes)
        Error::sys("writen error");
}

/* bytes remained in read buffer. */
//ssize_t SockStream::readlineBuf(void ** vptrptr)
//{}

/* Wrapper for readline. */
//ssize_t SockStream::Readline(void * ptr, size_t maxlen)
//{}
