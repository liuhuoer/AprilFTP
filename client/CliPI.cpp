#include "CliPI.h"

CliPI::CliPI(const char *host) : packet(this), readpacket(this)
{
    Socket cliSocket(CLI_SOCKET, host, CRRPORT);
    connfd = cliSocket.init();
    connSockStream.init(connfd);
}