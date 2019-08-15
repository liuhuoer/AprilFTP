#ifndef _CLIPI_H_
#define _CLIPI_H_

#include "../common/Common.h"
#include "../common/Socket.h"
#include "../common/SockStream.h"
#include "../common/Packet.h"
#include "CliDTP.h"

// Client Protocol Interpreter(CliPI)
class CliPI : public PI
{
public:
    CliPI(const char * host);   //#!!
    bool recvOnePacket();
    bool sendOnePacket(PacketStruct * ps, size_t nbytes);
    bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes);
    void run(uint16_t cmdid, std::vector<string> & cmdVector);
    string getEncodedParams(std::vector<string> & paramVector);

    bool cmdUSER(std::vector<string> & cmdVector);
    bool cmdPASS(std::vector<string> & cmdVector);
    void cmdGET(std::vector<string> & cmdVector);
    void cmdPUT(std::vector<string> & cmdVector);
    void cmdLS(std::vector<string> & cmdVector);

    bool confirmYN(const char * prompt);

    int getConnfd();

private:
    void saveUserState();

private:
    Packet          packet;
    Packet          readpacket;
    SockStream      connSockStream;
    string          userID;         //userID is equal to session ID
    int             connfd;
    static std::map<string, string> helpMap;
};

#endif