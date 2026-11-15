#include <iostream>
#include "socket_utils.h"
using namespace std;

int k_max_msg = 4096;
int32_t query(int client_fd, const char* buff)
{
    uint32_t length = (uint32_t) strlen(buff);
    if (length > k_max_msg)
    {
        msg("too long of a word");
        return -1;
    }

    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &length, 4);
    memcpy(&wbuf[4], buff, length);

    if (int32_t error = write_all(client_fd, wbuf, 4 + length))
        return error;

    char rbuf[4 + k_max_msg + 1];
    errno = 0;

    int32_t error = read_full(client_fd, rbuf, 4);
    if (error)
    {
        if (errno == 0)
            msg("EOF");

        else msg("read_full() error");

        return error;
    }

    memcpy(&length, rbuf, 4);
    if (length > k_max_msg)
    {
        msg("length too long");
        return -1;
    }

    error = read_full(client_fd, &rbuf[4], length);
    if (error)
    {
        msg("read_full() error");
        return error;
    }

    rbuf[4 + length] = '\0';
    cout << "server says: " << &rbuf[4] << endl;

    return 0;
}

int main(void)
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(client_fd, (const sockaddr*)& addr, sizeof(addr));
    if (rv)
        die("connect() error");

    int32_t error = query(client_fd, "HELLO");
    if (error)
        goto L_DONE;

    error = query(client_fd, "hello");
    if (error)
        goto L_DONE;

    error = query(client_fd, "HELLO");
    if (error)
        goto L_DONE;


    L_DONE:
        close(client_fd);
        return 0;
}