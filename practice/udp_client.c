#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])    // argv: ip + port
{
    // Check enough arguments
    if (argc != 3)
    {
        printf("Not enough arguments\n");
        exit(1);
    }

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == -1)
    {
        perror("Create Socket Failed\n");
        exit(1);
    }

    // Declare ip address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    char buff[512];
    while (1)
    {
        printf("Enter message (press enter to escape): ");
        fgets(buff, sizeof(buff), stdin);
        if (strncmp(buff, "\n", 1) == 0)
            break;
        int ret = sendto(clientSocket, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1)
        {
            printf("Send failed\n");
            break;
        }
        printf("%d bytes send\n", ret);
    }

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}