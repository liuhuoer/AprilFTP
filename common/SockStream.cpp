#include "SockStream.h"

/* Read "n" bytes from a fd. */
ssize_t SockStream::readn(void * vptr, size_t n)
{}

/* Wrapper for readn */
ssize_t SockStream::Readn(void * ptr, size_t nbytes)
{}

/* Wrapper for writen */
void SockStream::Writen(void * ptr, size_t nbytes)
{}

/* Write "n" bytes to a descriptor */
ssize_t SockStream::writen(const void * vptr, size_t n)
{}

/* bytes remained in read buffer. */
ssize_t SockStream::readlineBuf(void ** vptrptr)
{}

/* Wrapper for readline. */
ssize_t SockStream::Readline(void * ptr, size_t maxlen)
{}
