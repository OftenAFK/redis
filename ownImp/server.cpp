#include <iostream>
#include "socket_utils.h"
using namespace std;

const size_t k_max_msg = 4096;
static int32_t one_request(int client_fd)
{
    char rbuf[4 + k_max_msg + 1];
    errno = 0;

    int err = read_full(client_fd, rbuf, 4);
    if (err)
    {
        if (errno == 0)
            msg("EOF");

        else
            msg("read_full() error");

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);

    if (len > k_max_msg)
    {
        msg("Too big of a message");
        return -1;
    }

    err = read_full(client_fd, &rbuf[4], len);
    if (err)
    {
        msg("read_full() error");
        return -1;
    }

    rbuf[4 + len] = '\0';
    cout << "client says: " << &rbuf[4] << endl;

    const char reply[] = "world";
    len = (uint32_t) strlen(reply);
    char wbuf[4 + sizeof(reply)];

    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);


    return write_all(client_fd, wbuf, 4 + len);
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int value = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int rv = bind(fd, (const sockaddr*)& addr, sizeof(addr));
    if (rv)
        die("bind()");

    rv = listen(fd, SOMAXCONN);
    if (rv)
        die("listen()");

    while (true)
    {
        sockaddr_in client_addr = {};
        socklen_t len = sizeof(client_addr);

        int connfd = accept(fd, (struct sockaddr*)& client_addr, &len);
        if (connfd < 0)
            continue;

        while(true)
        {
            int32_t error = one_request(connfd);
            if (error)
                break;
        }

        close(connfd);
    }

    return 0;
}