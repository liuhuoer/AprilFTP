#ifndef _PACKET_H_
#define _PACKET_H_

#include "Common.h"
#include "Error.h"
#include "PI.h"

class Packet
{
public:
    Packet(PI * ppi);

    void fill(uint16_t tagid, uint16_t cmdid, uint16_t statid, uint16_t dataid, uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body);
    void fillCmd(uint16_t cmdid, uint16_t bsize, const char * body);

    void reset(PacketStoreType pstype);
    void savePacketState();

    // network byte order to host byte order
    void ntohp();
    void htonp();

    void sendCMD(uint16_t cmdid, string sbody);


    PacketStruct * getPs();
    uint32_t getSesid();
    uint16_t getTagid();
    uint16_t getCmdid();
    uint16_t getStatid();
    uint16_t getDataid();
    uint32_t getNslice();
    uint32_t getSindex();
    uint16_t getBsize();
    char * getBody();
    std::string getSBody();


private:
    PacketStoreType pstype;
    PacketStruct *ps;
    PacketStruct *prePs;
    PI * ppi;

};

#endif