#ifndef _COMMON_H_
#define _COMMON_H_

#include <netinet/in.h>     //sockaddr_in and other
#include <arpa/inet.h>      //inet(3) function
#include <sys/socket.h>

#include <cstdio>
#include <cstring>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#define DELIMITER   "\x1F"
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
    char body[PBODYCAP] // packet body
};

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
    STAT_OK = 1
};

enum DataID
{
    DATA_FILE = 1
};

/*****************************************************
 ********************** functions ********************
 ****************************************************/


#endif