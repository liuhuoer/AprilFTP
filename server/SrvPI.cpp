#include "SrvPI.h"

SrvPI::SrvPI(string dbFilename, int connfd) : packet(this), readpacket(this), db(DBFILENAME)
{
    this->connfd = connfd;
    connSockStream.init(connfd);
    sessionCommandPacketCount = 0;
    userID = "0";
    this->fp = NULL;
}

bool SrvPI::checkBreakpoint()
{
    std::map<string, string> selectParamMap = { {"USERID", this->userID}, 
                                                 {"SIZE", this->filesize},
                                                 {"ABSPATH", this->abspath},
                                                 {"VALID", "1"}};
    std::map<string, string> updateParamMap = { {"VALID", "0"}};
    if(db.selectNewest("ifile", selectParamMap))
    {
        vector< map<string, string>> resultMapVector = db.getResult();
        if(!resultMapVector.empty())
        {
            string body = resultMapVector[0]["NSLICE"] + DELIMITER + resultMapVector[0]["SINDEX"];
            packet.sendSTAT_BPR(body);
            recvOnePacket();
            if(packet.getTagid() == TAG_STAT &&packet.getStatid() == STAT_MD5)
            {
                string md5str = packet.getSBody();
                if(md5str == resultMapVector[0]["MD5SUM"])
                {
                    packet.sendSTAT_OK("Bingo...!! Breakpoint resumed");
                    db.update("ifile", resultMapVector[0]["ID"], updateParamMap);
                    SrvDTP srvDTP(&(this->packet), this);
                    srvDTP.recvFile(this->abspath.c_str(), std::stoul(resultMapVector[0]["NSLICE"]), std::stoul(resultMapVector[0]["SINDEX"]));
                    return true;
                }else{
                    packet.sendSTAT_FAIL("Breakpoint MD5SUM not math");
                    Error::msg("Breakpoint MD5SUM not match: \ncli: %s\nsrv: %s\n", md5str.c_str(), resultMapVector[0]["MD5SUM"].c_str());
                    return false;
                }
            }else{
                Error::msg("STAT_MD5: unknown tagid %d with statid %d", packet.getTagid(), packet.getStatid());
                return false;
            }
        }else{
            return false;
        }
    }else{
        Error::msg("checkBreakpoint: Database selectNewest error\n");
        return false;
    }
}

bool SrvPI::recvOnePacket()
{
    int n;
    packet.reset(NPACKET);
    if( (n = connSockStream.readn(packet.getPs(), PACKSIZE)) == 0)
    {
        this->saveUserState();
        Socket::tcpClose(connfd);
        Error::quit_pthread("client terminated prematurely");
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

bool SrvPI::sendOnePacketBlocked(PacketStruct * ps, size_t nbytes)
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

bool SrvPI::sendOnePacket(PacketStruct * ps, size_t nbytes)
{
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

        if(FD_ISSET(connfd, &rset))
        {
            readpacket.reset(NPACKET);
            if( (n = connSockStream.readn(readpacket.getPs(), PACKSIZE)) == 0)
            {
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::quit_pthread("client terminated prematurely");
            }else if(n < 0){
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::ret("connSockStream.readn() error");
                Error::quit_pthread("socket connection exception");
            }else{
                if(n == PACKSIZE)
                {
                    readpacket.ntohp();
                    printf("sendOnePacket method recive one packet: %s\n", readpacket.getSBody().c_str());
                }else{
                    printf("ERROR: sendOnePacket method recive one packet: n != PACKSIZE");
                }
            }
        }
        if(FD_ISSET(connfd, &wset))
        {
            if( (m = connSockStream.writen(ps, nbytes)) < 0 || (size_t)m != nbytes)
            {
                this->saveUserState();
                Socket::tcpClose(connfd);
                Error::ret("connSockStream.writen()");
                Error::quit_pthread("socket connection exception");
            }else{
                sendFlag = true;
            }
        }
    }
    return true;
}

void SrvPI::run()
{
    
    recvOnePacket();
    std::cout << "\n\n\033[32mNewCMD connfd: " << connfd << " [" << userRootDir << " " 
                << userRCWD << "]\033[0m" << std::endl;

    ++sessionCommandPacketCount;
    if(packet.getTagid() == TAG_CMD)
    {
        switch(packet.getCmdid())
        {
            case USER:
                cmdUSER();
                break;
            case PASS:
                cmdPASS();
                break;
            case PUT:
                cmdPUT();
                break;
            default:
                Error::msg("Server: sorry! this command function not finished yet.\n");
                break;
        }
    }else{
        Error::msg("Error: received packet is not a command.\n");
        packet.print();
        Error::quit_pthread("**********socket connection exception**********");
    }
}


void SrvPI::cmdUSER()
{
    printf("USER request\n");
    vector<string> paramVector;
    split(packet.getSBody(), "\t", paramVector);
    std::map<string, string> selectParamMap = { {"username", paramVector[0]} };

    if(db.select("user", selectParamMap))
    {
        vector< map<string, string>> resultMapVector = db.getResult();
        if(!resultMapVector.empty())
        {
            packet.sendSTAT_OK("this username exists");
        }else{
            packet.sendSTAT_ERR("no such username");
        }
    }else{
        packet.sendSTAT_ERR("Database select error");
    }
}

void SrvPI::cmdPASS()
{
    printf("PASS request\n");

    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);

    std::map<string, string> selectParamMap = { {"username", paramVector[0]}, {"password", paramVector[1]}};
    if(db.select("user", selectParamMap))
    {
        vector< map<string, string>> resultMapVector = db.getResult();
        if(!resultMapVector.empty())
        {
            // init userID, userRootDir, and userRCWD
            userID = resultMapVector[0]["ID"];
            userRootDir = ROOTDIR + resultMapVector[0]["USERNAME"];
            userRCWD = resultMapVector[0]["RCWD"];
            // set session ID
            packet.setSessionID(std::stoul(userID));
            packet.sendSTAT_OK("Welcome to AprilFTP.\n" \
                                + resultMapVector[0]["USERNAME"] \
                                + ", your last working directory is: ~" \
                                + userRCWD);
        }else{
            packet.sendSTAT_ERR("error: username mismatch password");
        }
    }else{
        packet.sendSTAT_ERR("Database select error");
    }
}

void SrvPI::cmdPUT()
{
    printf("PUT request\n");

    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    if(paramVector.size() == 2)
    {
        this->filesize = paramVector[1];
        paramVector.erase(paramVector.begin() + 1);
    }else if(paramVector.size() == 3){
        this->filesize = paramVector[2];
        paramVector.erase(paramVector.begin() + 2);
    }else{
        packet.sendSTAT_ERR("PUT params error");
        return;
    }

    this->clipath = paramVector[0];

    string userinput;
    if(paramVector.size() == 1)
    {
        vector<string> pathVector;
        split(paramVector[0], "/", pathVector);
        userinput = pathVector.back();
        this->filename = pathVector.back();
    }else if(paramVector.size() == 2){
        userinput = paramVector[1];

        vector<string> pathVector;
        split(paramVector[1], "/", pathVector);
        this->filename = pathVector.back();
    }

    string msg_o;
    int m;
    char buf[MAXLINE];
    SrvDTP srvDTP(&(this->packet), this);
    if( (m = combineAndValidatePath(PUT, userinput, msg_o, this->abspath)) < 0)
    {
        if(m == -2)
        {
            if(checkBreakpoint())
            {
                return;
            }
            packet.sendSTAT_CFM(msg_o.c_str());
            recvOnePacket();
            if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_CFM)
            {
                if(packet.getSBody() == "y")
                {
                    if(remove(this->abspath.c_str()) != 0)
                    {
                        packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
                        return;
                    }

                    if(sizecheck(this->filesize))
                    {
                        packet.sendSTAT_MD5(this->clipath + "\033[33mpreparing for flash transmission...\033[0m");
                        recvOnePacket();
                        if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_MD5)
                        {
                            string md5str = packet.getSBody();
                            if(md5check(md5str, this->abspath))
                            {
                                packet.sendSTAT_EOT("Flash transmission is done");
                                return;
                            }else{
                                srvDTP.recvFile(this->abspath.c_str(), 0, 0);
                                return;
                            }
                        }else{
                            Error::msg("STAT_MD5: unknown tagid %d with statid %d", packet.getTagid(), packet.getStatid());
                            return;
                        }
                    }else{
                        srvDTP.recvFile(this->abspath.c_str(), 0, 0);
                        return;
                    }
                }else{
                    return;
                }
            }else{
                Error::msg("STAT_CFM:unknown tagid %d with statid %d", packet.getTagid(), packet.getStatid());
                return;
            }
        }else{
            packet.sendSTAT_ERR(msg_o.c_str());
        }
    }else{
        std::cout<<"************cmdPUT path[" << this->abspath << "]" << '\n';
        if(sizecheck(this->filesize))
        {
            packet.sendSTAT_MD5(this->clipath + "\033[33mpreparing for flash transmission...\033[0m");
            recvOnePacket();
            if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_MD5)
            {
                string md5str = packet.getSBody();
                if(md5check(md5str, this->abspath))
                {
                    packet.sendSTAT_EOT("Flash transmission is done");
                    return;
                }else{
                    srvDTP.recvFile(this->abspath.c_str(), 0, 0);
                    return;
                }
            }else{
                Error::msg("STAT_MD5: unknown tagid %d with statid %d", packet.getTagid(), packet.getStatid());
                return;
            }
        }else{
            srvDTP.recvFile(this->abspath.c_str(), 0, 0);
            return;
        }
    }

    this->abspath.clear();
    this->filename.clear();
    this->filesize.clear();

}

bool SrvPI::sizecheck(string & sizestr)
{
    std::map<string, string> selectParamMap = { {"SIZE", sizestr}};
    if(db.select("file", selectParamMap))
    {
        vector< map<string, string>> resultMapVector = db.getResult();
        if(!resultMapVector.empty())
        {
            if(resultMapVector[0]["VALID"] == "1")
            {
                return true;
            }else{
                printf("\033[31msizechedk: this SIZE is not valid\033[0m\n");
                return false;
            }
        }else{
            printf("\033[31msizecheck: This SIZE not exist\033[0m\n");
            return false;
        }
    }else{
        Error::msg("\033[31mSIZE select error\033[0m\n");
        return false;
    }
}

bool SrvPI::md5check(string & md5str, string newpath)
{
    std::map<string, string> selectParamMap = { {"MD5SUM", md5str}};
    if(db.select("file", selectParamMap))
    {
        vector< map<string, string>> resultMapVector = db.getResult();
        if(!resultMapVector.empty())
        {
            if(link(resultMapVector[0]["ABSPATH"].c_str(), newpath.c_str()) < 0)
            {
                Error::ret("link");
                cerr << resultMapVector[0]["ABSPATH"] << ":" << newpath << endl;
                return false;
            }else{
                return false;
            }
        }else{
            printf("\033[31mMD5SUM not exist\033[0m\n");
            return false;
        }
    }else{
        Error::msg("\033[31mDatabase select error\033[0m\n");
        return false;
    }
}

int SrvPI::combineAndValidatePath(uint16_t cmdid, string userinput, string& msg_o, string & abspath_o)
{
    string newAbsPath;
    if(userinput.front() == '/')
    {
        string newAbsPath = userinput;
        abspath_o = newAbsPath;
        if(newAbsPath.substr(0, userRootDir.size()) != userRootDir)
        {
            msg_o = "Permission denied: " + newAbsPath;
            return -1;
        }else{
            if(cmdid == RMDIR && newAbsPath == userRootDir)
            {
                msg_o = "Permission denied: " + newAbsPath;
                return -1;
            }
            return cmdPathProcess(cmdid, newAbsPath, msg_o);
        }
    }
    string absCWD = userRootDir + userRCWD;

    vector<string> absCWDVector;
    split(absCWD, "/", absCWDVector);
    vector<string> userVector;
    split(userinput, "/", userVector);
    for(vector<string>::iterator iter = userVector.begin(); iter != userVector.end(); ++iter)
    {
        if(*iter == "..")
        {
            absCWDVector.pop_back();
        }else if(*iter == "."){
            continue;
        }else{
            absCWDVector.push_back(*iter);
        }
    }

    for(vector<string>::iterator iter = absCWDVector.begin(); iter != absCWDVector.end(); ++iter)
    {
        newAbsPath += "/" + *iter;
    }

    abspath_o = newAbsPath;
    std::cout << "newAbsPath: " << newAbsPath << '\n';
    if(newAbsPath.substr(0, userRootDir.size()) != userRootDir)
    {
        msg_o = "Permission denied: " + newAbsPath;
        return -1;
    }else{
        if(cmdid == RMDIR && newAbsPath == userRootDir)
        {
            msg_o = "Permission denied: " + newAbsPath;
            return -1;
        }
        return cmdPathProcess(cmdid, newAbsPath, msg_o);
    }
}

void SrvPI::saveUserState()
{
    std::cout << "\n\033[32mStart to save user state:\033[0m" << std::endl;

    if(fp != NULL)
    {
        cout << "unlock file and close fp in saveUserState\n" << endl;
        if( (flock(fileno(fp), LOCK_UN)) < 0)
        {
            Error::ret("flock");
        }
        Fclose(&fp);
    }

    map<string, string> updateParamMap = { {"RCWD", userRCWD}};
    db.update("user", userID, updateParamMap);
    packet.print();
    packet.pprint();
    if(packet.getPreTagid() == TAG_DATA && packet.getPreDataid() == DATA_FILE)
    {
        std::map<string, string> insertParamMap = { {"USERID", this->userID},
                                        {"ABSPATH", this->abspath},
                                        {"FILENAME", this->filename},
                                        {"SIZE", this->filesize},
                                        {"MD5SUM", md5sumNslice(this->abspath.c_str(), packet.getPreSindex())},
                                        {"NSLICE", packet.getPreSNslice()},
                                        {"SINDEX", packet.getPreSSindex()},
                                        {"SLICECAP", SSLICECAP}
                                        };
        db.insert("ifile", insertParamMap);
    }
    std::cout << "\n\033[32msave userstate ok\033[0m" << std::endl;
}