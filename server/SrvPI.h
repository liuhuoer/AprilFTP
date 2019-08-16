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
    bool checkBreakpoint();
    bool recvOnePacket();
    bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes);
    bool sendOnePacket(PacketStruct * ps, size_t nbytes);
    void run();

    void cmdUSER();
    void cmdPASS();
    void cmdPUT();
    void cmdGET();
    bool sizecheck(string & sizestr);
    bool md5check(string & md5str, string newpath);
    void cmdLS();
    void cmdCD();
    void cmdRM();
    void cmdPWD();
    void cmdMKDIR();
    void cmdSHELL();

    int getConnfd();
    FILE * setFp(FILE * fp);
    FILE * & getFp();
    Database * getPDB();
    string getClipath();
    string getFilename();
    unsigned long long getFilesize();

    ~SrvPI();

private:
    int sessionCommandPacketCount;
    Packet packet;
    Packet readpacket;
    int connfd;
    SockStream connSockStream;
    Database db;

    string userID;  // for simple, userID is equal to session ID
    string userRootDir;
    string userRCWD;   // current working directory relative to userRootDir

    string abspath;
    string filename;
    string filesize;

    string clipath;

    FILE * fp;

    int combineAndValidatePath(uint16_t cmdid, string userinput, string& msg_o, string & abspath_o);
    int cmdPathProcess(uint16_t cmdid, string newAbsPath, string & msg_o);
    void saveUserState();
};

#endif