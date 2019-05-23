#include "Error.h"

bool Error::daemon_proc = false;

/* Nofatal error related to syetem call
 * * Print message and return */
void Error::ret(const char * fmt, ...);
{
    va_list         ap;

    va_start(ap, fmt);
    doit(1, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/* Nonfatal error related to system call
 * * Print message and terminate*/
void Error::sys(const char * fmt, ...)
{
    va_list         ap;

    va_start(ap, fmt);
    doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    //!!
    pthread_exit((void *)1);
}

/* Nofatal error unrelated to system call
 * * Print message, and return*/
void Error::msg(const char * fmt, ...);
{
    va_list         ap;

    va_start(ap, fmt);
    doit(0, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/* Fatal error related to system call
 * * Print message and terminate*/
void Error::quit(const char * fmt, ...);
{
    va_list         ap;

    va_start(ap, fmt);
    doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    pthread_exit((void *)1);
}

void Error::quit_pthread(const char * fmt, ...);
{
    va_list         ap;

    va_start(ap, fmt);
    doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    pthread_exit((void *)1);
}

/* Print message and return to caller
 * * Caller specifies "errnoflag" and "level" */
void Error::doit(int errnoflag, int level, const char * fmt, va_list ap)
{
    int     errno_save, n;
    char    buf[MAXLINE + 1];
    char    errmsg[MAXLINE + 1];

    // value caller might want printed
    errno_save = errno;
#ifdef HAVE_VSnPRINTF
    //safe
    vsnprintf(buf, MAXLINE, fmt, ap);
#else
    //not safe
    vsprintf(buf, fmt, ap);
#endif
    n = strlen(buf);
    if(errnoflag)
    {
        snprintf(buf + n, MAXLINE - n, ": %s", strerror_r(errno_save, errmsg, MAXLINE));
    }
    strcat(buf, "\n");

    if(Error::daemon_proc)
    {
        syslog(level, "%s", buf);
        return;
    }else{
        // in case stdout and stderr are the same
        fflush(stdout);
        fputs(buf, stderr);
        fflush(stderr);
    }
    return;

}
