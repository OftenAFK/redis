#include <iostream>
#include "socket_utils.h"
using namespace std;

int main(void)
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        die("socket()");

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(client_fd, (const sockaddr *)& addr, sizeof(addr));
    if (rv)
        msg("connect() error");

    char wbuf[] = "client1 connected";
    write(client_fd, wbuf, sizeof(wbuf));

    char rbuf[64] = {};
    ssize_t n = read(client_fd, rbuf, sizeof(rbuf) - 1);

    close(client_fd);
}