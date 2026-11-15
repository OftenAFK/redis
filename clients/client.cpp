#include <iostream>
#include "socket_utils.h"
using namespace std;

static int32_t query(int fd, const char* text)
{
    uint32_t len = (uint32_t) strlen(text);
    // read the length of the text

    // make sure its not greater than the max allowed
    if (len > k_max_msg)
        return -1;


    // create a buffer, not using the null terminator here since we are writing into the buffer
    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    // copy the length into first 4 bytes of wbuf
    memcpy(&wbuf[4], text, len);
    // copy the text of length len after those 4 bytes

    if (int32_t err = write_all(fd, wbuf, 4 + len))
        return err;
    // if write_all returns err (-1), we return err

    // otherwise we build the reader buffer with a null terminator because here we do need to read
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    // errno is used to signify which type of errno has occured;
    int32_t err = read_full(fd, rbuf, 4);
    if (err)
    {
        if (errno == 0)
            msg("EOF");

        else
            msg("read() error");

        return err;
    }

    memcpy(&len, rbuf, 4);
    // reading length
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    err = read_full(fd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    rbuf[4 + len] = '\0';
    cout << "server says: " << &rbuf[4] << endl;
    // printing whatever we read
    return 0;
}

int main(void)
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        die("socket()");

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    int rv = connect(client_fd, (const struct sockaddr*)& addr, sizeof(addr));
    // this is where the client and server connect, on 127.0.0.1 : 1234
    if (rv)
        die("connect()");

    // char msg[] = "hello";
    // write(client_fd, msg, strlen(msg));

    // char rbuf[64] = {};
    // ssize_t n = read(client_fd, rbuf, sizeof(rbuf) - 1);
    // if (n < 0)
    //     die("read");

    // cout << "server says: " << rbuf << endl;
    // // close(client_fd);

    int32_t err = query(client_fd, "hello1");
    if (err)
        goto L_DONE;
    // goto L_DONE means jump straight to "this" line
    // this is for when an error occurs, we wont want either the server or the client to be stuck waiting infinitely
    // therefore when err occurs we jump to L_DONE which closes client_fd

    err = query(client_fd, "hello2");
    if (err)
        goto L_DONE;

    err = query(client_fd, "hello3");
    if (err)
        goto L_DONE;

    L_DONE:
        close(client_fd);
        return 0;
}

/*
    int main(void)
    {
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0)
            msg("error");

        sockaddr_in addr = {};
        addr.family = AF_INET;
        addr.port = hotns("1234");
        addr.sin_addr.s_addr = htohl(INADDR_LOOPBACK);

        int rv = connect(client_fd, (const sockaddr *)& addr, sizeof(addr));
        if (rv)
            die("connect failed");

        char msg[] = "Hello"
        write(client_fd, msg, strlen(msg));

        char rbuf[64] = {};
        int n = read(client_fd, rbuf, sizeof(rbuf) - 1);

        cout << "Server says: " << rbuf << endl;
    }
*/