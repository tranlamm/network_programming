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
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Create struct sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);;
    addr.sin_port = htons(atoi(argv[1]));

    // Bind socket to sockaddr
    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Binding Failed\n");
        exit(1);
    }

    // Listen 
    if (listen(serverSocket, 5) == -1)
    {
        printf("Listening Failed\n");
        exit(1);
    }
    printf("Waiting for client connecting ...\n");

    // Accept connection
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
    if (clientSocket == -1)
    {
        printf("Accept connection failed\n");
        exit(1);
    }
    printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
    // Receive message from server
    char buff[512];
    char computerName[128];
    char diskNumber;
    char diskName;
    short diskSize = 0;

    while (1)
    {
        int ret = recv(clientSocket, buff, sizeof(buff), 0);
        if (ret <= 0) break;
        printf("%d bytes receive\n", ret);

        int pos = 0;
        memcpy(&diskNumber, buff + pos, sizeof(diskNumber));
        pos += sizeof(diskNumber);

        memcpy(computerName, buff + 1 + atoi(&diskNumber) * 3, ret - 1 - atoi(&diskNumber) * 3);
        printf("Computer name: %s\n", computerName);
        printf("Numbers of disk: %c\n", diskNumber);

        for (int i = 0; i < atoi(&diskNumber); ++i)
        {
            memcpy(&diskName, buff + pos, sizeof(diskName));
            pos += sizeof(diskName);

            memcpy(&diskSize, buff + pos, sizeof(diskSize));
            pos += sizeof(diskSize);

            printf("%c - %hd\n", diskName, diskSize);
        }
    }

    // Close
    close(clientSocket);
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}