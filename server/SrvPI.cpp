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
            case GET:
                cmdGET();
                break;
            case LS:
                cmdLS();
                break;
            case CD:
                cmdCD();
                break;
            case RM:
                cmdRM();
                break;
            case PWD:
                cmdPWD();
                break;
            case MKDIR:
                cmdMKDIR();
                break;
            case SHELL:
                cmdSHELL();
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

void SrvPI::cmdGET()
{
    printf("GET request\n");

    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    cout << packet.getSBody() << ": " << paramVector[0] << endl;
    string srvpath;
    if(paramVector.size() == 1)
    {
        srvpath = paramVector[0];
    }else if(paramVector.size() == 2){
        srvpath = paramVector[0];
    }else{
        packet.sendSTAT_ERR("GET params error");
        return;
    }

    string msg_o;
    if(combineAndValidatePath(GET, paramVector[0], msg_o, this->abspath) < 0)
    {
        packet.sendSTAT_ERR(msg_o.c_str());
        return;
    }

    string path = this->abspath;
    SrvDTP srvDTP(&(this->packet), this);
    srvDTP.sendFile(path.c_str(), 0, 0);

    packet.sendSTAT_EOT();
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

void SrvPI::cmdLS()
{
    printf("LS request\n");
    char buf[MAXLINE];
    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    if(paramVector.size() == 0)
    {
        paramVector.push_back("");
    }
    string msg_o;
    if(combineAndValidatePath(LS, paramVector[0], msg_o, this->abspath) < 0)
    {
        packet.sendSTAT_ERR(msg_o.c_str());
        return;
    }
    string path = this->abspath;
    DIR * dir = opendir(path.c_str());
    if(!dir)
    {
        packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
        return;
    }else{
        packet.sendSTAT_OK();
    }
    struct dirent * e;
    int cnt = 0;
    int sindex = 0;
    string sbody;
    while( (e = readdir(dir)))
    {
        if(e->d_type == 4)
        {
            if(!strcmp(e->d_name, "..") || !strcmp(e->d_name, "."))
                continue;
            if(strlen(e->d_name) > 15)
            {
                if(sbody.empty() || sbody.back() == '\n')
                {
                    snprintf(buf, MAXLINE, "\033[36m%s\033[0m\n", e->d_name);
                }else{
                    snprintf(buf, MAXLINE, "\n\033[36m%s\033[0m\n", e->d_name);
                }
                cnt = 0;
            }else{
                snprintf(buf, MAXLINE, "\033[36m%-10s\033[0m\t", e->d_name);
                ++cnt;
            }
        }else{
            if(strlen(e->d_name) > 15)
            {
                if(sbody.empty() || sbody.back() == '\n')
                    snprintf(buf, MAXLINE, "%s\n", e->d_name);
                else
                    snprintf(buf, MAXLINE, "\n%s\n", e->d_name);
                cnt = 0;
            }else{
                snprintf(buf, MAXLINE, "%-10s\t", e->d_name);
                ++cnt;
            }
        }
        if(cnt != 0 && (cnt % 5) == 0)
            strcat(buf, "\n");
        
        if( (sbody.size() + strlen(buf)) > SLICECAP)
        {
            packet.sendDATA_LIST(0, ++sindex, sbody.size(), sbody.c_str());
            sbody.clear();
            recvOnePacket();
            if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_CTN)
                continue;
            else if(packet.getTagid() == TAG_STAT && packet.getStatid() == STAT_TERM)
                break;
            else{
                Error::msg("unknown packet");
                packet.print();
                return;
            }
        }
        sbody += buf;
    }
    if(!sbody.empty())
    {
        sindex = 0;
        if(sbody.back() == '\n')
            sbody.pop_back();
        packet.sendDATA_LIST(0, 0, sbody.size(), sbody.c_str());
    }
    packet.sendSTAT_EOT();
}

void SrvPI::cmdCD()
{
    printf("CD request\n");
    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    string msg_o;
    if(combineAndValidatePath(CD, paramVector[0], msg_o, this->abspath) < 0)
    {
        packet.sendSTAT_ERR(msg_o.c_str());
        return;
    }else{
        packet.sendSTAT_OK("CWD: ~" + userRCWD);
        return;
    }
}

void SrvPI::cmdRM()
{
    printf("RM request\n");
    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    string msg_o;

    vector<string>::iterator iter = paramVector.begin();
    if(paramVector[0][0] == '-')
    {
        if(combineAndValidatePath(RM, *iter, msg_o, this->abspath) < 0)
        {
            packet.sendSTAT_ERR(msg_o.c_str());
            return;
        }
        ++iter;
    }

    if(combineAndValidatePath(RM, *iter, msg_o, this->abspath) < 0)
    {
        packet.sendSTAT_ERR(msg_o.c_str());
        return;
    }else{
        char buf[MAXLINE];
        string path = this->abspath;
        if(remove(path.c_str()) != 0)
        {
            packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
            return;
        }else{
            packet.sendSTAT_OK(*iter + "is removed");
            return;
        }
    }
}

void SrvPI::cmdPWD()
{
    printf("PWD request\n");
    if(!packet.getSBody().empty())
    {
        if(packet.getSBody() == "-a")
            packet.sendSTAT_OK((userRootDir + userRCWD).c_str());
        else
            packet.sendSTAT_ERR("command format error");
    }else{
        packet.sendSTAT_OK(("~" + userRCWD).c_str());
    }
}

void SrvPI::cmdMKDIR()
{
    printf("MKDIR request\n");
    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);
    string msg_o;
    if(combineAndValidatePath(MKDIR, paramVector[0], msg_o, this->abspath) < 0)
    {
        packet.sendSTAT_ERR(msg_o.c_str());
        return;
    }else{
        string path = this->abspath;
        char buf[MAXLINE];
        if(mkdir(path.c_str(), 0777) == -1)
        {
            msg_o += "system call (mkdir): ";
            msg_o += strerror_r(errno, buf, MAXLINE);
            packet.sendSTAT_ERR(msg_o.c_str());
        }else{
            packet.sendSTAT_OK("Dir [" + paramVector[0] + "] created");
        }
    }
}

void SrvPI::cmdSHELL()
{
    printf("SHELL request\n");

    char buf[MAXLINE];
    vector<string> paramVector;
    split(packet.getSBody(), DELIMITER, paramVector);

    string curpath = userRootDir + (userRCWD == "/" ? "/" : userRCWD + "/");
    string shellCmdStr = "cd " + curpath + ";";
    auto it = paramVector.begin();
    shellCmdStr += *it;
    for(++it; it != paramVector.end(); ++it)
    {
        if((*it)[0] == '-')
            shellCmdStr += " " + *it;
        else{
            string msg_o;
            if(combineAndValidatePath(SHELL, *it, msg_o, this->abspath) < 0)
            {
                packet.sendSTAT_ERR(msg_o.c_str());
                return;
            }else{
                string path = userRootDir + (userRCWD == "/" ? "/" : userRCWD + "/") + *it;
                shellCmdStr += " " + path;
            }
        }
    }

    shellCmdStr += " 2>&1";
    cout << shellCmdStr << endl;
    FILE * fp = popen(shellCmdStr.c_str(), "r");
    //FILE * fp = popen("cd /home/AprilFTP/anonymous", "r");
    //FILE * fp_ = fopen("cd /home/AprilFTP/anonymous/delete", "w");
    if(fp == NULL)
    {
        packet.sendSTAT_ERR(strerror_r(errno, buf, MAXLINE));
        return;
    }

    char body[PBODYCAP] = {0};
    int n;
    packet.sendSTAT_OK();
    while( (n = fread(body, sizeof(char), PBODYCAP, fp)) > 0)
    {
        packet.sendDATA_TEXT(n, body);
    }
    pclose(fp);
    //fclose(fp_);
    packet.sendSTAT_EOT();
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

int SrvPI::cmdPathProcess(uint16_t cmdid, string newAbsPath, string & msg_o)
{
    string rpath = newAbsPath.substr(userRootDir.size(), newAbsPath.size() - userRootDir.size());
    if(rpath.empty())
    {
        rpath = "/";
    }
    switch(cmdid)
    {
        case GET:
        {
            struct stat statBuf;
            char buf[MAXLINE];
            int n = stat(newAbsPath.c_str(), &statBuf);
            if(!n)
            {
                if(S_ISREG(statBuf.st_mode))
                    return 0;
                else if(S_ISDIR(statBuf.st_mode)){
                    msg_o = "get: '" + newAbsPath + "' is a directory";
                    return -1;
                }else{
                    msg_o = "get: '" + newAbsPath + "' is not a directory";
                    return -1;
                }
            }else{
                msg_o = newAbsPath + strerror_r(errno, buf, MAXLINE);
                return -1;
            }
            break;
        }
        case RGET:
        {
            struct stat statBuf;
            char buf[MAXLINE];
            int n = stat(newAbsPath.c_str(), &statBuf);
            if(!n)
            {
                if(S_ISREG(statBuf.st_mode))
                {
                    msg_o = "rget: '" + newAbsPath + "' is a regular file";
                    return -1;
                }else if(S_ISDIR(statBuf.st_mode)){
                    return 0;
                }else{
                    msg_o = "rget: '" + newAbsPath + "' is not a directory";
                    return -1;
                }
            }else{
                msg_o = newAbsPath + strerror_r(errno, buf, MAXLINE);
                return -1;
            }
            break;
        }
        case PUT:
        {
            if( (access(newAbsPath.c_str(), F_OK)) == 0)
            {
                msg_o = "File '~" + rpath + "' already exists, overwrite ? (y/n)";
                return -2;
            }else{
                return 0;
            }
            break;
        }
        case RPUT:
        {
            if( (access(newAbsPath.c_str(), F_OK)) == 0)
            {
                msg_o = "File '~" + rpath + "' already exists, overwrite ? (y/n)";
                return -2;
            }else{
                return 0;
            }
            break;
        }
        case LS:
        {
            DIR * d = opendir(newAbsPath.c_str());
            char buf[MAXLINE];
            if(!d)
            {
                msg_o = strerror_r(errno, buf, MAXLINE);
                return -1;
            }else{
                closedir(d);
                return 0;
            }
            break;
        }
        case CD:
        {
            DIR * d = opendir(newAbsPath.c_str());
            char buf[MAXLINE];
            if(!d)
            {
                msg_o = strerror_r(errno, buf, MAXLINE);
                return -1;
            }else{
                //dir already exists
                closedir(d);
            }
            // update usereRCWD
            this->userRCWD = newAbsPath.substr(userRootDir.size(), newAbsPath.size() - userRootDir.size());
            if(this->userRCWD.empty())
            {
                this->userRCWD = "/";
            }
            return 0;
        }
        case RM:
        {
            struct stat statBuf;
            char buf[MAXLINE];
            int n = stat(newAbsPath.c_str(), &statBuf);
            string rpath = newAbsPath.substr(userRootDir.size(), newAbsPath.size() - userRootDir.size());
            if(rpath.empty())
            {
                rpath = "/";
            }
            if(!n)
            {
                if(S_ISREG(statBuf.st_mode))
                    return 0;
                else if(S_ISDIR(statBuf.st_mode)){
                    msg_o = "rm: '~" + rpath + "' is a directory";
                    return -1;
                }else{
                    msg_o = "rm: '~" + rpath + "' is not a regular file";
                    return -1;
                }
            }else{
                msg_o = strerror_r(errno, buf, MAXLINE);
                return -1;
            }
            break;
        }
        case MKDIR:
        {
            DIR * d = opendir(newAbsPath.c_str());
            if(!d)
                return 0;
            else{
                closedir(d);
                msg_o = "already exists: " + newAbsPath;
                return -1;
            }
            break;
        }
        case RMDIR:
        {
            struct stat statBuf;
            char buf[MAXLINE];
            int n = stat(newAbsPath.c_str(), &statBuf);
            if(!n)
            {
                if(S_ISREG(statBuf.st_mode))
                {
                    msg_o = "rmdir: '~" + rpath + "' is a regular file";
                    return -1;
                }else if(S_ISDIR(statBuf.st_mode)){
                    return 0;
                }else{
                    msg_o = "rmdir: '~" + rpath + "' is not a directory";
                    return -1;
                }
            }else{
                msg_o = strerror_r(errno, buf, MAXLINE);
                return -1;
            }
            break;
        }
        case SHELL:
        {
            if( (access(newAbsPath.c_str(), F_OK)) == 0)
                return 0;
            else{
                msg_o = "~" + rpath + ": No such file or directory";
                return -1;
            }
            break;
        }
        default:
        {
            msg_o = "SrvPI::cmdPathProcess: unknown cmdid";
            return -1;
            break;
        }
    }
    return -1;
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


int SrvPI::getConnfd()
{
    return connfd;
}

FILE * SrvPI::setFp(FILE * fp)
{
    this->fp = fp;
    return this->fp;
}

FILE * & SrvPI::getFp()
{
    return this->fp;
}

Database * SrvPI::getPDB()
{
    return &(this->db);
}

string SrvPI::getClipath()
{
    return this->clipath;
}

string SrvPI::getFilename()
{
    return this->filename;
}

unsigned long long SrvPI::getFilesize()
{
    return std::stoull(this->filesize);
}

SrvPI::~SrvPI()
{

}
