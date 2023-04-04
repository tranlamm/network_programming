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
        printf("Protocol: %d\n", p->ai_protocol);
        if (p->ai_family == AF_INET)
        {
            printf("IPv4: %s\n", inet_ntoa( ((struct sockaddr_in *)p->ai_addr)->sin_addr) );
            printf("Port: %d\n", ((struct sockaddr_in *)p->ai_addr)->sin_port);
        }
        else if (p->ai_family == AF_INET6)
        {
            char buf[64];
            printf("IPv6: %s\n", inet_ntop(p->ai_family, &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr, buf, p->ai_addrlen));
            printf("Port: %d\n", ((struct sockaddr_in6 *)p->ai_addr)->sin6_port);
        }
        p = p->ai_next;
    }

    freeaddrinfo(res);
    return 0;
}