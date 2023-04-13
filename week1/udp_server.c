#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])    // argv: port
{
    // Check enough arguments
    if (argc != 2)
    {
        printf("Not enough arguments\n");
        exit(1);
    }

    // Create socket
    int serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == -1)
    {
        perror("Create Socket Failed\n");
        exit(1);
    }

    // Bind socket to ip address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    int ret = bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf("Bind Failed\n");
        exit(1);
    }

    printf("Waiting for message ...\n");
    
    // Receive message from client
    char buff[512];
    struct sockaddr_in clientAddr;
    int size = sizeof(clientAddr);
    while (1)
    {
        int ret = recvfrom(serverSocket, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &size);
        if (ret <= 0)
        {
            printf("Receive failed\n");
            break;
        }
        if (ret < sizeof(buff))
            buff[ret] = 0;
        printf("%d bytes message from: %s:%d: %s", ret, inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port, buff);
    }

    // Close socket
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}