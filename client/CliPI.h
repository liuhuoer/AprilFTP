#ifndef _CLIPI_H_
#define _CLIPI_H_

#include "../common/Socket.h"
#include "../common/Packet.h"

class CliPI
{
public:
    CliPI(const char * host);   //#!!
private:
    Packet          packet;
    Packet          readpacket;
    int             connfd;
};

#endif