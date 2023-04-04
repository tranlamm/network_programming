#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    struct addrinfo *res, *p;           
    int ret = getaddrinfo(argv[1], "http", NULL, &res);

    if (ret != 0)
    {
        printf("Resolve Failed");
        return 1;
    }

    p = res;

    while (p != NULL)
    {
        if (p->ai_family == AF_INET)
        {
            printf("IPv4: %s\n", inet_ntoa( ((struct sockaddr_in *)p->ai_addr)->sin_addr) );
        }
        else if (p->ai_family == AF_INET6)
        {
            printf("IPv6\n");
        }
        p = p->ai_next;
    }

    freeaddrinfo(res);
    return 0;
}