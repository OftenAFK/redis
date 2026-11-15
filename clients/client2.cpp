#include <iostream>
#include "socket_utils.h"
using namespace std;

int main(void)
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        die("client_fd failed");

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    socklen_t addr_size = sizeof(addr);

    int rv = connect(client_fd, (const sockaddr *)& addr, addr_size);
    if (rv)
        die("rv failed");

    char message[] = "client2 connected";
    write(client_fd, message, sizeof(message));

    close(client_fd);
}