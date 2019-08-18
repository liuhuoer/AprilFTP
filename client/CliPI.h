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
    void cmdUSERADD(std::vector<string> & cmdVector);
    void cmdUSERDEL(std::vector<string> & cmdVector);

    void cmdGET(std::vector<string> & cmdVector);
    void cmdRGET(std::vector<string> & cmdVector);
    void cmdPUT(std::vector<string> & cmdVector);
    void cmdRPUT(std::vector<string> & cmdVector);
    void RPUT_iter(string srvRootPath, string cliRootPath);
    void cmdLS(std::vector<string> & cmdVector);
    void cmdLLS(std::vector<string> & cmdVector);
    void cmdCD(std::vector<string> & cmdVector);
    void cmdLCD(std::vector<string> & cmdVector);
    void cmdRM(std::vector<string> & cmdVector);
    void cmdLRM(std::vector<string> & cmdVector);
    void cmdPWD(std::vector<string> & cmdVector);
    void cmdLPWD(std::vector<string> & cmdVector);
    bool cmdMKDIR(std::vector<string> & cmdVector);
    void cmdLMKDIR(std::vector<string> & cmdVector);
    bool cmdLMKDIR(string path);
    void cmdSHELL(std::vector<string> & cmdVector);
    void cmdLSHELL(std::vector<string> & cmdVector);
    void cmdQUIT(std::vector<string> & cmdVector);
    void cmdHELP(std::vector<string> & cmdVector);

    bool confirmYN(const char * prompt);

    int getConnfd();

private:
    string toUpper(string & s);
    string toLower(string & s);
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