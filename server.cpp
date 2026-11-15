#include <iostream>
#include "socket_utils.h"
using namespace std;

static int32_t read_full(int fd, char* buff, size_t n)
{
    // the purpose of this is to read() until exactly n bytes have been recieved
    // since TCP is a byte stream, if the client sends hello then first read() might just read "hel" and the next one "lo"
    // read() inherently is "give me UP TO n bytes, not exactly n bytes"
    // what read_full() does is that it waits until n bytes are recieved
    while (n > 0)
    {
        ssize_t rv = read(fd, buff, n);
        if (rv <= 0)
            return -1;
            // error or unexpected EOF (End of File)

        assert((size_t) rv <= n);
        // asset checks if the condition passes, if not then it produces an error
        n -= (size_t) rv;
        // we then decrement the size of n by rv since we have read "rv" amount of bytes
        buff += rv;
        // the buffer pointer wouldve still pointed at the beginning even after adding data to it
        // therefore we do buff += rv to make sure the buff pointer points at the next empty space within buff
        // so no overwrite happens
    }
    // so whats happening here is that we run a loop for until n is greater than 0, we keep reading and
    // decrementing n by the amount we read until the loop exits at n <= 0

    return 0;
}

static int32_t write_all (int fd, const char* buff, size_t n)
{
    // same thing for write_all() we are trying to make it as such it only writes until it reaches n bytes
    while (n > 0)
    {
        ssize_t rv = write(fd, buff, n);
        if (rv <= 0)
            return -1;
            // again error

        assert((size_t) rv <= n);
        // asset checks if the condition passes, if it doesnt then it produces an error
        n -= (size_t) rv;
        buff += rv;
    }
    // running a loop until we have no more n to write (as in as long as n > 0)
    // we keep writing and then decrementing n by the amount we wrote

    return 0;
}

const size_t k_max_msg = 4096;
// handles exactly one complete request response at one time
static int32_t one_request(int connfd)
{
    char rbuf[4 + k_max_msg + 1];
    // bytes more than k_max_msg is not allowed, +4 for the header as in length of the message and +1 for the null terminator
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4); 
    // err here either stores 0 or -1, 0 if the message is read successfully and -1 when error occurs
    if (err)
    {
        if (errno == 0)
            msg("EOF");

        else
            msg("read() error");

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    // memcpy stands for memory copy and it works like memcpy(to, from, size)
    // here it would mean copy the number of bytes into len from rbuf of size 4
    // as in the first 4 bytes of rbuf would then be stored into variable len
    // and why uint_32? it stands for a 32 bit unsigned integer, which means it can hold only non negative numbers

    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }
    // we reject if len greater than the amount we can hold

    err = read_full(connfd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    rbuf[4 + len] = '\0';
    // adding the null terminator
    cout << "client says: " << &rbuf[4] << endl;
    // when cout recieves a *char it treats it as a c style string and prints until the null terminator

    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);

    memcpy(wbuf, &len, 4);
    // copy the length into the first 4 bytes of wbuf
    memcpy(&wbuf[4], reply, len);
    // copy the reply of length len into wbuf from 5th byte onwards

    return write_all(connfd, wbuf, 4 + len);
}

static void fd_set_nb(int fd)
{
    errno = 0;
    int flag = fcntl(fd, F_GETFL, 0);

    if (errno)
    {
        die("fcntl error");
        return;
    }

    flags |= O_NONBLOCK;

    errno = 0;
    (void) fcntl (fd, F_SETFL, flags);
    if (errno)
    {
        die("fcntl error")
    }
}

// perform_action handles one connected client using the connfd of that client
static void perform_action(int connfd)
{
    char rbuf[64] = {}; // creates a reciever buffer of 64 characters initialised to 0
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    // this tells the OS to read the data recieved from connfd and put it in rbuf
    // read(where_to_read_from, where_to_write, max_amount_to_be_read) (-1 for null terminator \0)
    // n is the number of bytes read
    if (n < 0) // if an error occures read returns -1 hence we used n < 0 condition
    {
        msg("read() error");
        return;
    }

    cout << "client says: " << rbuf << endl; // print the rbuf to know what the client said

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
    // this tells the OS to send the bytes in wbuf to the client with connfd where the number of bytes going
    // through would be strlen(wbuf)
}

int main(void)
{
    // setsockopt is used to configure various aspects of a socket, here we enables SO_REUSEADDR
    // without this the server wont be able to bind to the same address if restarted
    // value here means enable SO_REUSEADDR (if value == 1) otherwise turn it off (if value == 0)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // creates an ipv4 socket, AF_INET is ipv4, SOCK_STREAM for TCP and 0 for letting the OS choose the 
    // appropriate protocol for the requested socket type
    int value = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    // now we assign the IP and port using sockaddr_in addr
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;  // IPv4
    addr.sin_port = htons(1234);    // port 1234, htons stands for host to network short and converts integer to network
                                    // byte order
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // listen to any local network interface which does not limit it to only
                                                // 127.0.0.1, it can also listen to the machine's applicable local interfaces

    //bind
    int rv = bind(server_fd, (const sockaddr *)& addr, sizeof(addr));
    if (rv)
        die("bind()");

    //listen
    rv = listen(server_fd, SOMAXCONN); // SOMAXCON asks the OS to use its maximum/default supported backlog value
    if (rv)
        die("listen()");

    while(true) // infinite loop so the server does not die after just accepting one client
    {
        //accept
        sockaddr_in client_addr = {}; // preparing space for the client information
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(server_fd, (struct sockaddr *)& client_addr, &socklen);
        if (connfd < 0)
            continue;

        // perform_action(connfd);

        while (true)
        {
            int32_t error = one_request(connfd);
            if (error)
                break;
        }

        close(connfd);
    }
    return 0;
}

/*

    static void perform_action(int connfd)
    {
        char rbuf[64] = {};
        int n = read(connfd, rbuf, sizeof(rbuf) - 1);
        if (n < 0)
            msg("error");

        cout << "client says: " << rbuf << endl;

        char wbuf[64] = "World";
        write(connfd, wbuf, sizeof(wbuf));
    }
    
    int main(void)
    {
        int server_fd = socket(AF_INET, SOCKET_STREAM, 0);
        int value = 0;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

        sockaddr_in addr = {};
        addr.family = AF_INET;
        addr.port = htons(1234);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        int rv = bind(server_fd, (const sockaddr *)& addr, sizeof(addr));
        if (rv)
            dead("bind()");

        rv = listen(server_fd, SOMAXCONN);

        while(true)
        {
            sockaddr_in client_fd;
            socklen_t client_size = sizeof(client_fd);
            int connfd = accept(server_fd, (const sockadd *)& cleint_fd, &client_size);

            perform_action(connfd);
            close(connfd);
        }

        return 0;
    }
*/