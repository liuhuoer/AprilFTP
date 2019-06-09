#include "CliDTP.h"

CliDTP::CliDTP(Packet * ppacket, CliPI * pcliPI)
{
    this->ppacket = ppacket;
    this->pcliPI = pcliPI;
}

void CliDTP::sendFile(const char * pathname, FILE * fp, uint32_t nslice, uint32_t sindex, uint16_t slicecap)
{
    Packet & packet = *(this->ppacket);

    off64_t curpos = sindex * slicecap;
    if(lseek64(fileno(fp), curpos, SEEK_SET) < 0)
    {
        Error::ret("lseek64");
        return;
    }

    int n;
    char body[PBODYCAP];
    // int oldProgress = 0, newPreogress = 0;
    string hfilesize = getFileSizeString(pathname);
    if(nslice == 0)
    {
        fprintf(stderr, "\033[2K\r\033[0m%-40s%10s\t100%%", pathname, hfilesize.c_str());
    }else{
        while( (n = fread(body, sizeof(char), PBODYCAP, fp)) > 0)
        {
            packet.sendDATA_FILE(nslice, ++sindex, n, body);
        }
    }

    fclose(fp);
    packet.sendSTAT_EOF();

    while(pcliPI->recvOnePacket())
    {
        if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_PGS)
        {
            cerr << packet.getSBody();
        }else if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_WAIT){
            cerr << endl << packet.getSBody();
        }else if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_EOT){
            cout << packet.getSBody() << endl;
            break;
        }else{
            cout << "unknown packet" << endl;
            packet.print();
            break;
        }
    }
}

void CliDTP::recvFile(const char * pathname, FILE * fp, uint32_t nslice, uint32_t sindex, uint16_t slicecap)
{
    Packet & packet = *(this->ppacket);
    string hfilesize;
    int m;
    int oldProgress = 0, newProgress = 0;
    off64_t curpos = sindex * slicecap;
    if(lseek64(fileno(fp), curpos, SEEK_SET) < 0)
    {
        Error::ret("lseek64");
        return;
    }

    while(pcliPI->recvOnePacket())
    {
        switch(packet.getTagid())
        {
            case TAG_STAT:
            {
                switch(packet.getStatid())
                {
                    case STAT_OK:
                        break;
                    case STAT_SIZE:
                    {
                        if(std::stoull(packet.getSBody()) > getDiskAvailable())
                        {
                            packet.sendSTAT_ERR("insufficient disk space");
                            Error::msg("insufficient disk space");
                            return;
                        }else{
                            packet.sendSTAT_OK("sufficient disk space, ok to tranfer");
                        }
                        break;
                    }
                    case STAT_ERR:
                    {
                        cerr << packet.getSBody() << endl;
                        return;
                    }
                    case STAT_EOF:
                    {
                        fclose(fp);

                        std::cout << std::endl;
                        break;
                    }
                    case STAT_EOT:
                    {
                        return;
                    }
                    default:
                    {
                        Error::msg("unknown statid: %d", packet.getStatid());
                        break;
                    }
                }
                break;
            }
            case TAG_DATA:
            {
                switch(packet.getDataid())
                {
                    case DATA_FILE:
                    {
                        m = fwrite(packet.getBody(), sizeof(char), packet.getBsize(), fp);
                        if(m != packet.getBsize())
                        {
                            Error::msg("fwrite error %d/%d: %d vs %d Bytes\n", packet.getSindex(), packet.getNslice(), packet.getBsize(), m);
                            fclose(fp);
                            return;
                        }else{
                            if(packet.getNslice() == 0)
                            {
                                fprintf(stderr, "\033[2K\r\033[0m%-40s%10s\t100%%", pathname, hfilesize.c_str());
                                break;
                            }
                            newProgress = (packet.getSindex() * 1.0) / packet.getNslice() * 100;
                            if(newProgress > oldProgress)
                            {
                                fprintf(stderr, "\033[2K\r\033[0m%-40s%10s\t%3d%%", pathname, hfilesize.c_str(), newProgress);
                            }
                            oldProgress = newProgress;
                        }
                        break;
                    }
                    case DATA_TEXT:
                    {
                        hfilesize = packet.getSBody();
                        break;
                    }
                    default:
                    {
                        Error::msg("unknown statid: %d", packet.getStatid());
                        break;
                    }
                }
                break;
            }
            default:
            {
                Error::msg("unknown tagid: %d", packet.getTagid());
                break;
            }
        }
    }

}