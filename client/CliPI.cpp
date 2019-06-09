#include "CliPI.h"


std::map<string, string> CliPI::helpMap = {
                {"USERADD", "useradd -u [username] -p [password]"},
                {"USERDEL", "userdel [username]"},

                {"GET",     "get [remote-file] [local-file]"},
                {"PUT",     "put [local-file]  [remote-file]"}
};

CliPI::CliPI(const char *host) : packet(this), readpacket(this)
{
    Socket cliSocket(CLI_SOCKET, host, CTRPORT);
    connfd = cliSocket.init();
    connSockStream.init(connfd);
}

//!!
bool CliPI::recvOnePacket()
{
    int n;
    packet.reset(NPACKET);
    printf("CliPI::recvOnePacket1\n");
    if( (n = connSockStream.readn(packet.getPs(), PACKSIZE)) == 0)
    {
        this->saveUserState();
        Socket::tcpClose(connfd);
        Error::quit("server terminated prematurely");
    }else if(n < 0){
        this->saveUserState();
        Socket::tcpClose(connfd);
        Error::ret("connSockStream.readn() error");
        Error::quit_pthread("socket connection exception");
    }else{
        packet.ntohp();
    }
    return true;
}
//end!!
bool CliPI::sendOnePacketBlocked(PacketStruct * ps, size_t nbytes)
{
    int m;
    if( (m = connSockStream.writen(ps, nbytes)) < 0 || (size_t)m != nbytes)
    {
        this->saveUserState();
        Socket::tcpClose(connfd);
        Error::ret("connSockStream.writen()");
        Error::quit_pthread("socket connection exception");
        return false;
    }else{
        return true;
    }
}

bool CliPI::sendOnePacket(PacketStruct * ps, size_t nbytes)
{
    cout << "CliPI::sendONePacket1" << endl;
    int n, m;
    bool sendFlag = false;
    int maxfdp1;
    fd_set rset, wset;

    FD_ZERO(&rset);
    FD_ZERO(&wset);

    while(!sendFlag)
    {
        FD_SET(connfd, &rset);
        FD_SET(connfd, &wset);

        maxfdp1 = connfd + 1;
        if(select(maxfdp1, &rset, &wset, NULL, NULL) < 0)
        {
            this->saveUserState();
            Socket::tcpClose(connfd);
            Error::ret("select error");
            Error::quit_pthread("socket connection exception");
        }

        //socket is readable
        if(FD_ISSET(connfd, &rset))
        {
            readpacket.reset(NPACKET);
            if( (n = connSockStream.readn(readpacket.getPs(), PACKSIZE)) == 0)
            {
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::quit_pthread("server terminated prematurely");
            }else if(n < 0){
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::ret("connSocketStream.readn() error");
                Error::quit_pthread("socket  connection exception");
            }else{
                if(n == PACKSIZE)
                {
                    readpacket.ntohp();
                    if(readpacket.getTagid() == TAG_STAT && readpacket.getStatid() == STAT_PGS)
                    {
                        cerr << readpacket.getSBody();
                    }
                }else{
                    printf("ERROR: sendOnePacket method recive one packet: n != PACKSIZE");
                }
            }
        }
        cout << "CliPI::sendONePacket2" << endl;
        //socket is writable
        if(FD_ISSET(connfd, &wset))
        {
            cout << "CliPI::sendONePacket2.1" << endl;
            if( (m = connSockStream.writen(ps, nbytes)) < 0 || (size_t)m != nbytes)
            {
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::ret("connSockStream.writen()");
                Error::quit_pthread("socket connection exception");
            }else{
                cout << "CliPI::sendONePacket2.3" << endl;
                sendFlag = true;
            }
        }
        cout << "CliPI::sendONePacket3" << endl;
    }
    return true;
}
 

void CliPI::run(uint16_t cmdid, std::vector<string> & paramVector)
{
#ifdef debug
    printf("*****\nCliPI::run()_0\n*****\n");
#endif

    switch(cmdid)
    {
        case USER:
            cmdUSER(paramVector);
            break;
        case PASS:
            cmdPASS(paramVector);
            break;
        case GET:
#ifdef debug
    printf("*****\nCliPI::run()_1\n*****\n");
#endif
            cmdGET(paramVector);
            break;
        case PUT:
            cmdPUT(paramVector);
            break;
        default:
            Error::msg("Client: Sorry! this command function not finished yet.\n");
            break;
    }
}

string CliPI::getEncodedParams(std::vector<string> & paramVector)
{
    string encodedParams;
    if(!paramVector.empty())
    {
        /*
        for(vector<string>::iterator iter = paramVector.begin(); iter != paramVector.end(); ++iter)
        {
            encodedParams += DELIMITER + *iter;
        }
        */
        vector<string>::iterator iter = paramVector.begin();
        encodedParams += *iter;
        for(++iter; iter != paramVector.end(); ++iter)
        {
            encodedParams += DELIMITER + *iter;
        }
    }

    return encodedParams;
}

bool CliPI::cmdUSER(std::vector<string> & paramVector)
{
    if(paramVector.empty() || paramVector.size() != 1)
    {
        Error::msg("Usage: [username]");
        return false;
    }else{
        packet.sendCMD(USER, getEncodedParams(paramVector));
        // first receive response
        recvOnePacket();
        if(packet.getTagid() == TAG_STAT)
        {
            if(packet.getStatid() == STAT_OK)
                return true;
            else if(packet.getStatid() == STAT_ERR){
                cerr << packet.getSBody() << endl;
                return false;
            }else{
                Error::msg("CliPI::cmdUSER: unknown statid %d", packet.getStatid());
                return false;
            }
        }else{
            Error::msg("CliPI::cmdUSER: unknown tagid %d", packet.getTagid());
            return false;
        }
    }
}

bool CliPI::cmdPASS(std::vector<string> & paramVector)
{
    printf("CliPI::cmdPASS1\n");
    if(paramVector.empty() || paramVector.size() != 2)
    {
        Error::msg("Usage: [password]");
        for(vector<string>::iterator iter = paramVector.begin(); iter != paramVector.end(); ++iter)
        {
            std::cout << *iter << '\n';
        }
        return false;
    }
    printf("CliPI::cmdPASS2\n");
    paramVector[1] = encryptPassword(paramVector[1]);
    printf("CliPI::cmdPASS2.1\n");
    packet.sendCMD(PASS, getEncodedParams(paramVector));

    paramVector[1] = encryptPassword(paramVector[1]);
    //first receive response
    recvOnePacket();
    printf("CliPI::cmdPASS3\n");
    if(packet.getTagid() == TAG_STAT)
    {
        if(packet.getStatid() == STAT_OK)
        {
            //init userID, same as session id
            char buf[MAXLINE];
            //!!
            snprintf(buf, MAXLINE, "%u", packet.getSesid());
            userID = buf;
            cout << packet.getSBody() << endl;
            return true;
        }else if(packet.getStatid() == STAT_ERR)
        {
            cerr << packet.getSBody() << endl;
            return false;
        }else{
            Error::msg("CliPI::cmdPASS: unkown statid %d", packet.getStatid());
            return false;
        }
    }else{
        Error::msg("CliPI::cmdPASS: unknown tagid %d", packet.getTagid());
        return false;
    }
}

void CliPI::cmdGET(std::vector<string> & paramVector)
{
    if(paramVector.empty() || paramVector.size() > 2)
    {
        cout << "Usage: " << helpMap["GET"] << endl;
        return;
    }

    string pathname;
    uint32_t nslice = 0;
    uint32_t sindex = 0;

    char buf[MAXLINE];
    if(paramVector.size() == 1)
    {
        vector<string> pathVector;
        split(paramVector[0], "/", pathVector);
        pathname = pathVector.back();
    }else if(paramVector.size() == 2){
        pathname = paramVector[1];
    }

    if( (access(pathname.c_str(), F_OK)) == 0)
    {
        snprintf(buf, MAXLINE, "File [%s] already exists, overwrite ? (y/n)", pathname.c_str());
        if(!confirmYN(buf))
            return;
    }
    FILE *fp;
    if( (fp = fopen(pathname.c_str(), "wb")) == NULL)
    {
        Error::msg("%s", strerror_r(errno, buf, MAXLINE));
        return;
    }else{
        packet.sendCMD(GET, getEncodedParams(paramVector));
    }

    CliDTP cliDTP(&(this->packet), this);
    cliDTP.recvFile(pathname.c_str(), fp, nslice, sindex);
}

void CliPI::cmdPUT(std::vector<string> & paramVector)
{
    if(paramVector.empty() || paramVector.size() > 2)
    {
        std::cout << "Usage: " << helpMap["PUT"] << std::endl;
        return;
    }
    char pathname[MAXLINE];
    char buf[MAXLINE];
    uint32_t nslice = 0;
    uint32_t sindex = 0;

    strcpy(pathname, paramVector[0].c_str());
    struct stat statBuf;
    int n = stat(paramVector[0].c_str(), &statBuf);
    if(!n)
    {
        if(S_ISREG(statBuf.st_mode))
        {
            ;
        }else if(S_ISDIR(statBuf.st_mode)){
            cout << "put: cannot upload [" << pathname << "]: Is a directory" << endl;
            return;
        }else{
            cout << "put: [" << pathname << "] not a regular file or directory" << endl;
            return;
        }
    }else{
        Error::msg("stat: %s", strerror_r(errno, buf, MAXLINE));
        return;
    }

    FILE * fp;
    if( (fp = fopen(pathname, "rb")) == NULL)
    {
        Error::msg("%s", strerror_r(errno, buf, MAXLINE));
        return;
    }else if( (n = getFileNslice(pathname, &nslice)) < 0){
        if(n == -2){
            Error::msg("Too large file size.", buf);
        }else{
            Error::msg("File stat error.");
        }
        return;
    }else{
        //first check file size
        string sizestr = getFilesize(string(pathname));
        if(sizestr.empty())
        {
            Error::ret("getFilesize error");
            return;
        }
        paramVector.push_back(sizestr);
        packet.sendCMD(PUT, getEncodedParams(paramVector));
    }

    while(recvOnePacket())
    {
        switch(packet.getTagid())
        {
            case TAG_CMD:
            {
                switch(packet.getCmdid())
                {
                    case GET:
                        break;
                    case LMKDIR:
                        break;
                    default:
                        Error::msg("unknown cmdid: %d", packet.getCmdid());
                        break;
                }
                break;
            }
            case TAG_STAT:
            {
                switch(packet.getStatid())
                {
                    case STAT_OK:
                    {
                        CliDTP cliDTP(&(this->packet), this);
                        cliDTP.sendFile(pathname, fp, nslice, sindex);
                        return;
                    }
                    case STAT_BPR:
                    {
                        vector<string> paramVector;
                        split(packet.getSBody(), DELIMITER, paramVector);
                        cout << "File size match: " << paramVector[1] << "/" << paramVector[0] << endl;
                        uint32_t tmp_sindex = std::stoul(paramVector[1]);

                        string md5str = visualmd5sumNslice(pathname, tmp_sindex);
                        if(md5str.empty())
                        {
                            printf("md5sum error\n");
                            return;
                        }
                        packet.sendSTAT_MD5(md5str);
                        recvOnePacket();
                        if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_OK)
                        {
                            sindex = tmp_sindex;
                            printf("\033[32mBreakpoint resumed: [%s %u/%u]\033[0m\n", pathname, sindex, nslice);
                        }else if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_FAIL){
                            cout << packet.getSBody() << endl;
                        }else{
                            printf("packet error\n");
                            packet.print();
                            return;
                        }
                        break;
                    }
                    case STAT_MD5:
                    {
                        cout << packet.getSBody() << endl;
                        string md5str = visualmd5sum(pathname);
                        if(md5str.empty())
                        {
                            printf("md5sum error\n");
                            return;
                        }
                        packet.sendSTAT_MD5(md5str);
                        break;
                    }
                    case STAT_CFM:
                    {
                        if(confirmYN(packet.getSBody().c_str()))
                        {
                            packet.sendSTAT_CFM("y");
                        }else{
                            packet.sendSTAT_CFM("n");
                            return;
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
                        cout << packet.getSBody() << endl;
                        break;
                    }
                    case STAT_EOT:
                    {
                        cout << packet.getSBody() << endl;
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
                        cout << "DATA_FILE" << packet.getSBody() << endl;
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

bool CliPI::confirmYN(const char * prompt)
{
    string inputline, word;
    vector<string> paramVector;
    while(printf("%s", prompt), getline(cin, inputline))
    {
        paramVector.clear();
        std::istringstream is(inputline);
        while(is >> word)
            paramVector.push_back(word);
        
        if(paramVector.size() == 1)
        {
            if(paramVector[0] == "y")
                return true;
            else if(paramVector[0] == "n")
                return false;
            else
                continue;
        }else
            continue;
    }
    return false;
}

int CliPI::getConnfd()
{
    return this->connfd;
}


void CliPI::saveUserState()
{
    std::cout << "\n\033[32msave user state ok\033[0m" << std::endl;
}
