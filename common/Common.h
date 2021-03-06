#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/types.h>
#include <sys/stat.h>       //mkdir()
#include <sys/statfs.h>     //struct statfs
#include <sys/file.h>
#include <netinet/in.h>     //sockaddr_in and other
#include <arpa/inet.h>      //inet(3) function
#include <netdb.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <termios.h>
#include <dirent.h>         //Dir, opendir() and closedir()

#include <unistd.h>         //select() and FD_ZERO, FD_SET and close()

#include <errno.h>

#include <cstdio>
#include <cstring>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <queue>
#include <sstream>
using namespace std;

#define DEBUG       1

#define DBFILENAME  "AprilFTP.db"
#define ROOTDIR     "/home/AprilFTP/"
#define ROOTDIR_LEN strlen(ROOTDIR)
#define KERNELDIR   "/home/AprilFTP/.AprilFTP/"
#define GHOSTDIR    "/home/AprilFTP/.AprilFTP/ghost/"

#define PASSSALT0   "&5@f#fe)"
#define PASSSALT1   "@AprilFTP"

#define DELIMITER   "\x1F"

// MACRO constants
#define LISTENQ         1024            // 2nd argument(backlog) to listen()

// Miscellaneous constants
#define MAXLINE         256             // max text line length

#define CTRPORT         2121            // server: control listening port
#define DATPORT         2020            // server: data listening port


#define SA sockaddr

class Database;

struct ThreadArg
{
    int fd;
    uint32_t sesid;

    char buf[MAXLINE];
    Database * pdb;
};

enum SockType
{
    SRV_SOCKET,
    CLI_SOCKET
};

/*****************************************************
 ********************** packet ***********************
 ****************************************************/

enum PacketStoreType
{
    HPACKET,            // Host storage type
    NPACKET             // Network storage type
};

/********************** PacketStruct ****************/
#define PHEADSIZE       20              //packet header size
#define PBODYCAP        512             //packet body capacity
#define SLICECAP        512             //slice capcacity
#define SSLICECAP       "512"           //slice capcacity
#define MAXNSLICE       4284967295      //max slice count

#define PACKETSIZE      sizeof(PacketSturct)

struct PacketStruct
{
    // packet header
    uint32_t sesid;     // Session id
    uint16_t tagid;     // different packet type: CMD, DATA, INFO
    
    uint16_t cmdid;     // Command: ID
    uint16_t statid;    // status code id
    uint16_t dataid;    // data type id

    uint32_t nslice;    // Data: whole number of file slices
    uint32_t sindex;    // Data: slice index

    uint16_t bsize;     // the real size of body
    // packet body
    char body[PBODYCAP];// packet body
};

#define PACKSIZE sizeof(PacketStruct)

enum TagID
{
    TAG_CMD = 1,
    TAG_STAT,
    TAG_DATA
};

enum CmdID
{
    USER = 1,
    PASS,
    USERADD,
    USERDEL,

    GET,
    PUT,
    LS,
    LLS,        //local ls
    CD,
    LCD,
    RM,
    LRM,
    PWD,
    LPWD,
    MKDIR,
    LMKDIR,
    QUIT,
    HELP,

    MGET,
    MPUT,
    RGET,
    RPUT,
    RMDIR,

    SHELL,
    LSHELL,
};

enum StatID
{
    STAT_OK = 1,
    STAT_BPR,   // breakpoint resume
    STAT_CFM,   //confirm
    STAT_MD5,   //md5sum
    STAT_PGS,   //progress
    STAT_FAIL,  //fail
    STAT_ERR,   //error
    STAT_CTN,   //continue
    STAT_TERM,  //terminate
    STAT_SIZE,  //size
    STAT_WAIT,  //wait
    STAT_EOF,   //end of file
    STAT_EOT,   //end of transfer
};

enum DataID
{
    DATA_FILE = 1,
    DATA_TEXT,
    DATA_LIST,
    DATA_NAME,
    DATA_OTHER
};

/*****************************************************
 ********************** functions ********************
 ****************************************************/
void Fclose(FILE *fp);
void Fclose(FILE **fp);


void * Malloc(size_t size);

void Pthread_create(pthread_t *, const pthread_attr_t *,
                    void * (*)(void *), void *);

string size2str(unsigned long filesize);
int getFileNslice(const char * pathname, uint32_t * pnslice_o);
string getFileSizeString(const char * pathname);

string md5sum(const char * str);
string md5sum(const char * str, int len);
string md5sumNslice(const char * pathname, uint32_t nslice);
string visualmd5sum(const char * pathname);
string visualmd5sumNslice(const char * pathname, uint32_t nslice);
string encryptPassword(string password);
string getCurrentTime();
unsigned long long getFilesize(const char * pathname);
string getFilesize(string pathname);

unsigned long long getDiskAvailable();

void restore_terminal_settings(void);
void disable_terminal_return(void);

// tools
void split(std::string src, std::string token, vector<string> & vect);

string getInode(const char * pathname);

#endif