#ifndef _PI_H_
#define _PI_H_

class PI
{
public:
    virtual bool recvOnePacket() = 0;
    virtual bool sendOnePacket(PacketStruct * ps, size_t nbytes) = 0;
    virtual bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes) = 0;
};

#endif