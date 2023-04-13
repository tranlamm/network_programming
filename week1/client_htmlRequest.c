#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    // Check enough arguments
    if (argc != 2)
    {
        printf("Missing arguments\n");
        exit(1);
    }

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // DNS
    // argv[1] = http://httpbin.org/net
    char *domain = NULL;
    char *path = NULL;
    int length;
    char *tmp = strchr(argv[1] + 7, '/');
    if (tmp == NULL)
    {
        length = strlen(argv[1]) - 7;
        domain = malloc(length + 1);
        memcpy(domain, argv[1] + 7, length);
        domain[length] = 0;
        path = malloc(2);
        path = "/";
    }
    else
    {
        length = tmp - argv[1] - 7;
        domain = malloc(length + 1);
        memcpy(domain, argv[1] + 7, length);
        domain[length] = 0;
        int path_length = strlen(argv[1]) - length - 7;
        path = malloc(path_length + 1);
        memcpy(path, argv[1] + 7 + length, path_length);
        path[path_length] = 0;
    }

    struct addrinfo *result;
    int ret = getaddrinfo(domain, "http", NULL, &result);
    if (ret != 0)
    {
        perror("DNS Failed\n");
        exit(1);
    }

    // Connect to server
    if (connect(clientSocket, result->ai_addr, result->ai_addrlen) == -1)
    {
        printf("Connecting failed\n");
        exit(1);
    }

    // Send request to server
    char msg[256];
    sprintf(msg, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);
    send(clientSocket, msg, strlen(msg), 0);

    free(domain);
    free(path);

    // Receive message from server
    char buff[1028];
    while (1)
    {
        if(recv(clientSocket, buff, sizeof(buff), 0) <= 0)
            break;

        printf("%s", buff);
    }

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}