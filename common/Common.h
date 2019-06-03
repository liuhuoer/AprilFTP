#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/types.h>
#include <sys/stat.h>       //mkdir()
#include <sys/file.h>
#include <netinet/in.h>     //sockaddr_in and other
#include <arpa/inet.h>      //inet(3) function
#include <netdb.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <dirent.h>         //Dir, opendir() and closedir()

#include <unistd.h>         //select() and FD_ZERO, FD_SET and close()


#include <cstdio>
#include <cstring>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#define DEBUG       1

#define DBFILENAME  "AprilFTP.db"
#define ROOTDIR     "/home/AprilFTP"
#define ROOTDIR_LEN strlen(ROOTDIR)
#define KERNELDIR   "/home/AprilFTP/.AprilFTP/"
#define GHOSTDIR    "/home/AprilFTP/.AprilFTP/ghost"

#define PASSSALT0   "&5@f#fe)"
//#define PASSSALT1   "@AprilFTP"
#define PASSSALT1   "@tinyFTP"

#define DELIMITER   "\x1F"

// MACRO constants
#define LISTENQ         1024            // 2nd argument(backlog) to listen()

// Miscellaneous constants
#define MAXLINE         256             // max text line length

#define CTRPORT         2121            // server: control listening port
#define DATPORT         2020            // server: data listening port


#define SA sockaddr

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
    PASS
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
};

enum DataID
{
    DATA_FILE = 1
};

/*****************************************************
 ********************** functions ********************
 ****************************************************/
void Fclose(FILE *fp);
void Fclose(FILE **fp);


void * Malloc(size_t size);
string md5sum(const char * str, int len);
string md5sumNslice(const char * pathname, uint32_t nslice);
string encryptPassword(string password);


#endif