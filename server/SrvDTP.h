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
};


#endif