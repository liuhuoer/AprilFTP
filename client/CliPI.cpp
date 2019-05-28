#include "CliPI.h"

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
        //socket is writable
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
 

void CliPI::run(uint16_t cmdid, std::vector<string> & paramVector)
{
    switch(cmdid)
    {
        case USER:
            cmdUSER(paramVector);
            break;
        case PASS:
            cmdPASS(paramVector);
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
        for(vector<string>::iterator iter = paramVector.begin(); iter != paramVector.end(); ++iter)
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
    if(paramVector.empty() || paramVector.size() != 2)
    {
        Error::msg("Usage: [password]");
        for(vector<string>::iterator iter = paramVector.begin(); iter != paramVector.end(); ++iter)
        {
            std::cout << *iter << '\n';
        }
        return false;
    }
    paramVector[1] = encryptPassword(paramVector[1]);
    packet.sendCMD(PASS, getEncodedParams(paramVector));

    //first receive response
    recvOnePacket();
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

int CliPI::getConnfd()
{
    return this->connfd;
}


void CliPI::saveUserState()
{
    std::cout << "\n\033[32msave user state ok\033[0m" << std::endl;
}