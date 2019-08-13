#include "UI.h"
#include <cstdio>

map<const string, const uint16_t> UI::cmdMap = {
    {"USER",        USER},
    {"PASS",        PASS},
    {"PUT",         PUT},
    {"GET",         GET}
};


UI::UI(const char *host) : cliPI(host)
{

}

void UI::run()
{
    string word;
    string inputline;

    //log-in
    while(printf("Username for 'AprilFTP':"), getline(std::cin, inputline))
    {
        this->cmdVector.clear();

        std::istringstream is(inputline);
        while(is >> word)
            this->cmdVector.push_back(word);
        
        if(this->cmdVector.empty())
        {
            this->cmdVector.push_back("anonymous");
            this->cmdVector.push_back("anonymous");
            if(!cliPI.cmdPASS(this->cmdVector))
            {
                continue;
            }else{
                break;
            }
        }

        if(!cliPI.cmdUSER(this->cmdVector))
        {
            continue;
        }else{
            char *password = getpass("\033[35mPassword for 'AprilFTP': \033[0m");
            this->cmdVector.push_back(password);
            if(!cliPI.cmdPASS(this->cmdVector))
            {
                continue;
            }else{
                break;
            }
        }
    }

    this->username = this->cmdVector[0];

    //!!
    int             maxfdp1;
    fd_set          rset;
    int connfd = cliPI.getConnfd();
    
    FD_ZERO(&rset);

    printf("%s@AprilFTP>", username.c_str());
    while(1)
    {
        fflush(stdout);
        FD_SET(connfd, &rset);
        FD_SET(fileno(stdin), &rset);
        maxfdp1 = connfd + 1;
        if(select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
            Error::sys("select error");
        
        if(FD_ISSET(connfd, &rset))
            cliPI.recvOnePacket();

        if(FD_ISSET(fileno(stdin), &rset))
        {
            getline(std::cin, inputline);
            cmdRun(inputline);
            printf("%s@AprilFTP>", username.c_str());
        }
    }
    //end!!
}

void UI::cmdRun(string & inputline)
{
    this->cmdVector.clear();

    for(auto it = inputline.begin(); it < inputline.end(); ++it)
    {
        string param;
        for(; it < inputline.end(); ++it)
        {
            if((*it) == ' ' || (*it) == '\t')
            {
                break;
            }else if((*it) == '\\' && (it + 1) != inputline.end() && *(it + 1) == ' '){
                param += ' ';
                ++it;
            }else{
                param += *it;
            }
        }
        if(!param.empty())
        {
            this->cmdVector.push_back(param);
        }
    }
    if(!cmdCheck())
    {
        return;
    }else{
        cmdVector.erase(cmdVector.begin());
        cliPI.run(this->cmdid, this->cmdVector);
    }
}

bool UI::cmdCheck()
{
    if(cmdVector.empty())
        return false;

    map<const string, const uint16_t>::iterator iter = cmdMap.find(toUpper(cmdVector[0]));
    if(iter != cmdMap.end())
    {
        this->cmdid = iter->second;
        return true;
    }else{
        std::cerr << cmdVector[0] << ": command not found" << std::endl;
        return false;
    }
}

string UI::toUpper(string &s)
{
    string upperStr;
    for(string::size_type i = 0; i < s.size(); ++i)
        upperStr += toupper(s[i]);
    return upperStr;
}