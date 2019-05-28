#ifndef _SRVPI_H_
#define _SRVPI_H_

#include "../common/Common.h"
#include "../common/Error.h"
#include "../common/Packet.h"
#include "../common/Socket.h"
#include "../common/SockStream.h"
#include "../common/Database.h"
#include "../common/PI.h"
#include "SrvDTP.h"

class SrvPI : public PI
{
public:
    SrvPI(string dbFilename, int connfd);
    bool recvOnePacket();
    bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes);
    bool sendOnePacket(PacketStruct * ps, size_t nbytes);
    void run();
    void split(std::string src, std::string token, vector<string> & vect);

    void cmdUSER();
    void cmdPASS();

private:
    int sessionCommandPacketCount;
    Packet packet;
    Packet readpacket;
    int connfd;
    SockStream connSockStream;
    Database db;

    string userID;  // for simple, userID is equal to session ID
    std::string userRootDir;
    std::string userRCWD;   // current working directory relative to userRootDir

    string abspath;
    string filename;
    string filesize;

    FILE * fp;

    void saveUserState();
};

#endif