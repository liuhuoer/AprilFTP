#include "server.h"
#include "../common/database.h"


int main(int argc, char ** argv)
{
    {
        Database db(DBFILENAME);
        db.init();
    }
    printf("MAXSLICE: %lu\n", MAXSLINE);
    struct sockaddr_in cliaddr;
}