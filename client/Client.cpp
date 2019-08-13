#include "UI.h"
#include "../common/Error.h"
#include <cstdio>
#include <string.h>

int main(int argc, char **argv)
{
    //input IPaddress and port number
    if(argc != 2)
        Error::quit("usage: ./client <IPaddress>");
    UI clientUI(argv[1]);   //#!!
    clientUI.run();         //#!!
}