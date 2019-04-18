#ifndef _UI_H_
#define _UI_H_

#include <vector>
#include <string>
#include "CliPI.h"
using namespace std;

typedef struct command
{
private:
    short int               id;
    std::vector<string>     argVec;
}Command;

class UI
{
public:
    UI(const char * host);          //#!!
    void                     run(); //#!!
private:
    uint16_t                 cmdid;
    CliPI                    cliPI;
};

#endif