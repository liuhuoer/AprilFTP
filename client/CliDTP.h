#ifndef _CLIDTP_H_
#define _CLIDTP_H_

#include "../common/Common.h"
#include "../common/Error.h"
#include "../common/Packet.h"
#include "../common/Socket.h"
#include "../common/SockStream.h"
#include "CliPI.h"


class CliPI;
// Data Transfer Process
class CliDTP
{
public:
    CliDTP(Packet * ppacket, CliPI * pcliPI);
    void recvOnePacket();

    void sendFile(const char * pathname, FILE * fp, uint32_t nslice, uint32_t sindex, uint16_t slicecap = SLICECAP);
    void recvFile(const char * filename, FILE * fp, uint32_t nslice, uint32_t sindex, uint16_t slicecap = SLICECAP);

    void removeFile(const char * pathname);

private:
    Packet * ppacket;
    CliPI * pcliPI;
};


#endif