#ifndef _CLIPI_H_
#define _CLIPI_H_

#include "../common/Socket.h"
#include "../common/Packet.h"

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

private:
    void saveUserState();

private:
    Packet          packet;
    Packet          readpacket;
    SockStream      connSockStream;
    string          userID;         //userID is equal to session ID
    int             connfd;
};

#endif