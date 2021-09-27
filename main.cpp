
#include <stdio.h>

#include "tcpserver.h"

int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        printf("Arguments:\n");
        printf("				-tun Tun Device name\n");
        printf("				-port listening port number\n");
    }

    return 0;

}
