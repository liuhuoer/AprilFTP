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

void Packet::fillStat(uint16_t statid, uint16_t bsize, const char * body)
{
    fill(TAG_STAT, 0, statid, 0, 0, 0, bsize, body);
}

void Packet::fillData(uint16_t dataid, uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body)
{
    fill(TAG_DATA, 0, 0, dataid, nslice, sindex, bsize, body);
}
 

void Packet::setSessionID(uint32_t sesid)
{
    ps->sesid = sesid;
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

    prePs->nslice = ps->nslice;
    prePs->sindex = ps->nslice;
    prePs->bsize = ps->bsize;
}

void Packet::sendCMD(uint16_t cmdid, string sbody)
{
    // send OPHEADSIZEK
    this->reset(HPACKET);
    this->fillCmd(cmdid, sbody.size(), sbody.c_str());
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendDATA_FILE(uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body)
{
    this->reset(HPACKET);
    this->fillData(DATA_FILE, nslice, sindex, bsize, body);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
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

    ps->nslice = ntohl(ps->nslice);
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

    ps->nslice = htonl(ps->nslice);
    ps->sindex = htonl(ps->sindex);
    ps->bsize = htons(ps->bsize);

    this->pstype = NPACKET;
}

void Packet::print()
{
    if(!DEBUG)
        return;
    
    if(pstype == HPACKET)
    {
        printf("\t\t[HOST Packet: %p]\n", ps);
    }else if(pstype ==NPACKET){
        printf("\t\t[NETWORK Packet: %p]\n", ps);
    }else{
        Error::msg("unknown PacketStoreType\n");
        return;
    }

    printf("\t\tsesid = %u\n", ps->sesid);
    printf("\t\ttagid = %d\n", ps->tagid);
    printf("\t\tcmdid = %d\n", ps->cmdid);
    printf("\t\tstatid = %d\n", ps->statid);
    printf("\t\tdataid = %d\n", ps->dataid);
    printf("\t\tnslice = %u\n", ps->nslice);
    printf("\t\tsindex = %u\n", ps->sindex);
    printf("\t\tbsize = %d\n", ps->bsize);
    if(pstype == HPACKET)
    {
        printf("\t\tbody = %s\n", this->getSBody().c_str());
    }

    fflush(stdout);
}

void Packet::pprint()
{
    printf("\t\t[Previous HOST Packet: %p]\n", prePs);
    printf("\t\tsesid = %u\n", ps->sesid);
    printf("\t\ttagid = %d\n", ps->tagid);
    printf("\t\tcmdid = %d\n", ps->cmdid);
    printf("\t\tstatid = %d\n", ps->statid);
    printf("\t\tdataid = %d\n", ps->dataid);
    printf("\t\tnslice = %u\n", ps->nslice);
    printf("\t\tsindex = %u\n", ps->sindex);
    printf("\t\tbsize = %d\n", ps->bsize);

    fflush(stdout);
}

void Packet::sendSTAT_OK()
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[32mOK to transfer\033[0m");
    this->fillStat(STAT_OK, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_OK(const char * msg)
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[32m%s\033[0m", msg);
    this->fillStat(STAT_OK, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);

}

void Packet::sendSTAT_OK(string msg)
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[32m%s\033[0m", msg.c_str());
    this->fillStat(STAT_OK, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);

}

void Packet::sendSTAT_MD5(string body)
{
    this->reset(HPACKET);
    this->fillStat(STAT_MD5, body.size(), body.c_str());
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_CFM(const char * msg)
{
    // send CFM
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "%s", msg);
    this->fillStat(STAT_CFM, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_CFM(string msg)
{
    // send CFM
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "%s", msg.c_str());
    this->fillStat(STAT_CFM, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}



void Packet::sendSTAT_ERR()
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[31mError occurred\033[0m");
    this->fillStat(STAT_ERR, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_ERR(const char * msg)
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg);
    this->fillStat(STAT_ERR, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_ERR(string msg)
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg.c_str());
    this->fillStat(STAT_ERR, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOF()
{
    //send EOF
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[32mEnd of file\033[0m");
    this->fillStat(STAT_EOF, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOF(string msg)
{
    this->reset(HPACKET);
    char buf[MAXLINE];
    snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg.c_str());
    this->fillStat(STAT_EOF, strlen(buf), buf);
    this->htonp();
    ppi->sendOnePacket(this->ps, PACKSIZE);
}



PacketStruct * Packet::getPs()
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

uint16_t Packet::getStatid()
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


PacketStruct * Packet::getPrePs()
{
    return prePs;
}

uint32_t Packet::getPreSesid()
{
    return prePs->sesid;
}

uint16_t Packet::getPreTagid()
{
    return prePs->tagid;
}

uint16_t Packet::getPreCmdid()
{
    return prePs->cmdid;
}

uint16_t Packet::getPreStatid()
{
    return prePs->statid;
}

uint16_t Packet::getPreDataid()
{
    return prePs->dataid;
}

uint32_t Packet::getPreNslice()
{
    return prePs->nslice;
}


string Packet::getPreSNslice()
{
    char buf[MAXLINE] = {0};
    snprintf(buf, MAXLINE, "%u", prePs->nslice);
    return string(buf);
}

uint32_t Packet::getPreSindex()
{
    return prePs->sindex;
}

string Packet::getPreSSindex()
{
    char buf[MAXLINE] = {0};
    snprintf(buf, MAXLINE, "%u", prePs->sindex);
    return string(buf);
}

uint16_t Packet::getPreBsize()
{
    return prePs->bsize;
}

