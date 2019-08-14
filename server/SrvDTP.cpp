#include "SrvDTP.h"

SrvDTP::SrvDTP(Packet * ppacket, SrvPI * psrvPI)
{
    this->ppacket = ppacket;
    this->psrvPI = psrvPI;
}


void SrvDTP::insertNewFileMD5SUM(const char * pathname, Database * pdb)
{
    string inode = getInode(pathname);

    cout << "md5sum computing..." << endl;
    string md5str = md5sum(pathname);
    string sizestr = getFilesize(string(pathname));
    cout << "insertNewFileMD5SUM # filepath: " << pathname << "md5str: " << md5str
            << "sizestr: " << sizestr << endl;
    
    string ghostfilename;
    ghostfilename += getCurrentTime();
    ghostfilename += "_" + md5str + "_";
    ghostfilename += psrvPI->getFilename();
    string ghostPath = GHOSTDIR + ghostfilename;

    if(!md5str.empty() && !sizestr.empty())
    {
        std::map<string, string> selectParamMap = { {"MD5SUM", md5str}};
        if(pdb->select("file", selectParamMap))
        {
            vector< map<string, string>> resultMapVector = pdb->getResult();
            if(resultMapVector.empty())
            {
                std::map<string, string> insertParamMap = { {"MD5SUM", md5str},
                                                            {"MD5RAND", "NULL"},
                                                            {"ABSPATH", ghostPath},
                                                            {"FILENAME", ghostfilename},
                                                            {"INODE", inode},
                                                            {"SIZE", sizestr}};
                if(pdb->insert("file", insertParamMap))
                {
                    Error::msg("Success: insert new file MD5SUM");
                    if(link(pathname, ghostPath.c_str()) < 0)
                    {
                        Error::ret("\033[31mlink\033[0m");
                        cerr << pathname << ":" << ghostPath << endl;
                    }
                }else{
                    Error::msg("\033[31mDatabase insert error\033[0m");
                }
            }else{
                Error::msg("\033[31mThis MD5SUM already exists\033[0m");
            }
        }else{
            Error::msg("\033[31mDatabse select error\033[0m");
        }
    }
}

void SrvDTP::recvFile(const char *pathname, uint32_t nslice, uint32_t sindex, uint16_t slicecap)
{
    Packet & packet = *(this->ppacket);
    char buf[MAXLINE];

    if(psrvPI->getFilesize() > getDiskAvailable())
    {
        packet.sendSTAT_ERR("insufficient disk space");
        return;
    }

    if(psrvPI->setFp(fopen(pathname, "ab")) == NULL)
    {
        packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
        return;
    }else{
        if( (flock(fileno(psrvPI->getFp()), LOCK_EX | LOCK_NB)) < 0)
        {
            Error::ret("flock");
            packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
            return;
        }

        off64_t n;
        off64_t curpos = sindex * slicecap;
        if( (n = lseek64(fileno(psrvPI->getFp()), curpos, SEEK_SET)) < 0)
        {
            packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
            return;
        }else{
            printf("Recv file [%s %u/%u] now\n", pathname, sindex, nslice);
            packet.sendSTAT_OK();
        }
    }
    int m;
    int oldProgress = 0, newProgress = 0;
    string hfilesize = size2str(psrvPI->getFilesize());
    while(psrvPI->recvOnePacket())
    {
        if(packet.getTagid() == TAG_DATA && packet.getDataid() == DATA_FILE)
        {
            m = fwrite(packet.getBody(), sizeof(char), packet.getBsize(), psrvPI->getFp());
            if(m != packet.getBsize())
            {
                Error::msg("Received slice %d/%d: %d vs %d Bytes\n", packet.getSindex(), packet.getNslice(), packet.getBsize(), m);
                return;
            }

            newProgress = (packet.getSindex()*1.0) / packet.getNslice() * 100;
            if(newProgress > oldProgress)
            {
                snprintf(buf, MAXLINE, "\033[2K\r\033[0m%-40s%10s\t%3d%%", psrvPI->getClipath().c_str(), hfilesize.c_str(), newProgress);
                packet.sendSTAT_PGS(buf);
            }
            oldProgress = newProgress;
        }else if(packet.getTagid() == TAG_STAT)
        {
            if(packet.getStatid() == STAT_EOF)
            {
                if( (flock(fileno(psrvPI->getFp()), LOCK_UN)) < 0)
                {
                    Error::ret("flock");
                }
                Fclose(&psrvPI->getFp());
                std::cout << packet.getSBody() << std::endl;

                if(psrvPI->getFilesize() > (256 * 1024 * 1024))
                {
                    packet.sendSTAT(STAT_WAIT, "MD5 computing...");
                }
                
                insertNewFileMD5SUM(pathname, psrvPI->getPDB());

                printf("EOT [%s]\n", pathname);
                packet.sendSTAT_EOT("Done");

                return;
            }else if(packet.getStatid() == STAT_EOT){
                std::cout  << packet.getSBody() << std::endl;
                return;
            }else{
                Error::msg("SrvDTP::recvFile TAG_STAT: unknown statid %d", packet.getStatid());
                return;
            }
        }else{
            Error::msg("SrvDTP::recvFile: unknown tagid %d with statid %d", packet.getTagid(), packet.getStatid());
            return;
        }
    }
}