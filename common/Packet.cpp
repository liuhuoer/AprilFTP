#include "Packet.h"

Packet::Packet(PI * ppi)
{
    this->pstype = HPACKET;
    ps = (PacketStruct*)Malloc(PACKSIZE);
    prePs = (PacketStruct*)Malloc(PACKSIZE);
    ps->sesid = 0;
    this->ppi = ppi;
}

void Packet::fill(uint16_t tagid, uint16_t cmdid, uint16_t statid, uint16_t dataid, 
                    uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body)
{
    ps->tagid = tagid;
    
    ps->cmdid = cmdid;
    ps->statid = statid;
    ps->dataid = dataid;

    ps->nslice = nslice;
    ps->sindex = sindex;
    ps->bsize = bsize;

    if(bsize > PBODYCAP)
    {
        Error::msg("\033[31mPacket::fill error: bsize = %d, bsize > PBODYCAP\033[0m", bsize);
        return;
    }
    if(body != NULL && bsize != 0)
    {
        memcpy(ps->body, body, bsize);
    }
}

void Packet::fillCmd(uint16_t cmdid, uint16_t bsize, const char * body)
{
    fill(TAG_CMD, cmdid, 0, 0, 0, 0, bsize, body);
}

// !!
void Packet::reset(PacketStoreType pstype)
{
    //must keep sesid
    if(this->pstype == NPACKET)
    {
        if(pstype == HPACKET)
            ps->sesid = ntohl(ps->sesid);
    }else if(this->pstype == HPACKET){
        this->savePacketState();
        if(pstype == NPACKET)
            ps->sesid = ntohl(ps->sesid);
    }
    this->pstype = pstype;

    ps->tagid = 0;

    ps->cmdid = 0;
    ps->statid = 0;
    ps->dataid = 0;

    ps->nslice = 0;
    ps->sindex = 0;
    ps->bsize = 0;

    memset(ps->body, 0, PBODYCAP);
}
// end!!

void Packet::savePacketState()
{
    prePs->sesid = ps->sesid;

    prePs->tagid = ps->tagid;

    prePs->cmdid = ps->cmdid;
    prePs->statid = ps->statid;
    prePs->dataid = ps->dataid;

    pre->nslice = ps->nslice;
    pre->sindex = ps->nslice;
    pre->bsize = ps->bsize;
}

void sendCMD(uint16_t cmdid, string sbody)
{
    // send OPHEADSIZEK
    this->reset(HPACKET);
    this->fillCmd(cmdid, sbody.size(), sbody.c_str());
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKETSIZE);
}

void Packet::ntohp()
{
    if(pstype == HPACKET)
    {
        Error::msg("already in HOST byte order\n");
        return;
    }
    ps->sesid = ntohl(ps->sesid);
    ps->tagid = ntohs(ps->tagid);

    ps->cmdid = ntohs(ps->cmdid);
    ps->statid = ntohs(ps->statid);
    ps->dataid = ntohs(ps->dataid);

    ps->nslice = ntohl(ps->slice);
    ps->sindex = ntohl(ps->sindex);
    ps->bsize = ntohs(ps->bsize);

    this->pstype = HPACKET;
}

void Packet::htonp()
{
    if(pstype == NPACKET)
    {
        Error::msg("already in NETWORK byte order\n");
        return;
    }
    ps->sesid = htonl(ps->sesid);
    ps->tagid = htons(ps->tagid);

    ps->cmdid = htons(ps->cmdid);
    ps->statid = htons(ps->statid);
    ps->dataid = htons(ps->dataid);

    ps->nslice = htonl(ps->slice);
    ps->sindex = htonl(ps->sindex);
    ps->bsize = htons(ps->bsize());

    this->pstype = NPACKET;
}


PacketStruct * packet::getPs()
{
    return ps;
}

uint32_t Packet::getSesid()
{
    return ps->sesid;
}

uint16_t Packet::getTagid()
{
    return ps->tagid;
}

uint16_t Packet::getCmdid()
{
    return ps->cmdid;
}

uint16_t Packet::Statid()
{
    return ps->statid;
}

uint16_t Packet::getDataid()
{
    return ps->dataid;
}

uint32_t Packet::getNslice()
{
    return ps->nslice;
}

uint32_t Packet::getSindex()
{
    return ps->sindex;
}

uint16_t Packet::getBsize()
{
    return ps->bsize;
}

char * Packet::getBody()
{
    return ps->body;
}

std::string Packet::getSBody()
{
    char buf[PBODYCAP + 1] = {0};
    strncpy(buf, ps->body, ps->bsize);
    return string(buf);
}