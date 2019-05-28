#include "SrvPI.h"

SrvPI::SrvPI(string dbFilename, int connfd) : packet(this), readpacket(this), db(DBFILENAME)
{
    this->connfd = connfd;
    connSockStream.init(connfd);
    sessionCommandPacketCount = 0;
    userID = "0";
    this->fp = NULL;
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
{}

bool SrvPI::sendOnePacket(PacketStruct * ps, size_t nbytes)
{}

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

void SrvPI::split(std::string src, std::string token, vector<string> & vect)
{
    int nbegin = 0;
    int nend = 0;
    while(nend != -1 && (unsigned int)nbegin < src.length())
    {
        nend = src.find_first_of(token, nbegin);
        if(nend == -1)
        {
            vect.push_back(src.substr(nbegin, src.length() - nbegin));
        }else{
            if(nend != nbegin)
            {
                vect.push_back(src.substr(nbegin, nend - nbegin));
            }
        }
        nbegin = nend + 1;
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