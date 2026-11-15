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