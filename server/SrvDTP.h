#ifndef _SRVDTP_H_
#define _SRVDTP_H_

#include "../common/Common.h"
#include "../common/Error.h"
#include "../common/Packet.h"
#include "../common/Socket.h"
#include "../common/SockStream.h"
#include "SrvPI.h"

class SrvPI;
// Server Data Transfer Process(SrvDTP)
class SrvDTP
{
public:

    SrvDTP(Packet * ppacket, SrvPI * psrvPI);

    void insertNewFileMD5SUM(const char * pathname, Database * pdb);
    void recvFile(const char *pathname, uint32_t nslice, uint32_t sindex, uint16_t slicecap = SLICECAP);

private:
    Packet * ppacket;
    SrvPI * psrvPI;
};


#endif