#include "ui.h"
#include <cstdio>

int main(int argc, char **argv)
{
    //input IPaddress and port number
    if(argc != 2)
        printf("error_ip");//err

    UI clientUI(argv[1]);   //#!!
    clientUI.run();         //#!!
}