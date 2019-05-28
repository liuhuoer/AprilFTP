#ifndef _ERROR_H_
#define _ERROR_H_

#include "Common.h"
#include <stdarg.h>     //ANSI C header file 
#include <syslog.h>     //for syslog() 

class Error
{
public:
    /* Nofatal error related to syetem call
     * * Print message and return */
    static void ret(const char * fmt, ...);
    /* Nonfatal error related to system call
     * * Print message and terminate*/
    static void sys(const char * fmt, ...);
    /* Nofatal error unrelated to system call
     * * Print message, and return*/
    static void msg(const char * fmt, ...);
    /* Fatal error related to system call
     * * Print message and terminate*/
    static void quit(const char * fmt, ...);

    static void quit_pthread(const char * fmt, ...);

private:
    /* Print message and return to caller
    * * Caller specifies "errnoflag" and "level" */
    static void doit(int, int, const char*, va_list);
    /* set true by daemon_init()*/
    static bool daemon_proc;
};

#endif