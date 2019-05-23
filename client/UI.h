#ifndef _UI_H_
#define _UI_H_

#include <vector>
#include <string>
#include "CliPI.h"
using namespace std;

struct Command
{
private:
    short int               id;
    std::vector<string>     argVec;
};

// User Interface
class UI
{
public:
    UI(const char * host);          //#!!
    void                    run(); //#!!
private:
    std::vector<string>     cmdVector;
    uint16_t                cmdid;
    CliPI                   cliPI;
    string                  username;

    static map<const string, const uint16_t> cmdMap;

    void                    cmdRun(string &);
    bool                    cmdCheck();
    string                  toUpper(string &);
};

#endif