#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;
// helper functions
void die(const char* msg)
{
    perror(msg);
    exit(1);
}

void msg(const char* msg)
{
    cout << msg << endl;
}

const size_t k_max_msg = 4096;
static int32_t read_full(int fd, char* buff, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buff, n);

        if (rv <= 0)
            return -1;

        assert((size_t)rv <= n);

        n -= (size_t)rv;
        buff += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const char* buff, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buff, n);

        if (rv <= 0)
            return -1;

        assert((size_t)rv <= n);

        n -= (size_t)rv;
        buff += rv;
    }

    return 0;
}