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
    if (argc != 3)
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

    // Connect to server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    
    if (connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Connecting failed\n");
        exit(1);
    }

    // Send message to server
    char buff[512];
    char computerName[128];
    char diskNumber;
    int pos = 0;

    printf("Enter computer name: ");
    scanf("%s", computerName);
    while(getchar() != '\n');

    printf("Enter numbers of disk: ");
    scanf("%c", &diskNumber);
    while(getchar() != '\n');
    
    memcpy(buff + pos, &diskNumber, sizeof(diskNumber));
    pos += sizeof(diskNumber);

    char diskName;
    short diskSize;
    for (int i = 0; i < atoi(&diskNumber); ++i)
    {
        printf("Enter disk name: ");
        scanf("%c", &diskName);
        while(getchar() != '\n');

        printf("Enter disk size: ");
        scanf("%hd", &diskSize);
        while(getchar() != '\n');

        memcpy(buff + pos, &diskName, sizeof(diskName));
        pos += sizeof(diskName);

        memcpy(buff + pos, &diskSize, sizeof(diskSize));
        pos += sizeof(diskSize);
    }
    memcpy(buff + pos, computerName, strlen(computerName));

    int ret = send(clientSocket, buff, 1 + strlen(computerName) + atoi(&diskNumber)*3, 0);
    printf("%d bytes send\n", ret);

    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}