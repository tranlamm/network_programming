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

    // Receive hello message from server
    char buff[512];
    char *message = NULL;
    int size = 0;
    while (1)
    {
        int ret = recv(clientSocket, buff, sizeof(buff), 0);
        if (ret <= 0)
        {
            printf("Receive message failed\n");
            exit(1);
        }
        message = realloc(message, size + ret);
        memcpy(message + size, buff, ret);
        size += ret;
        if (strstr(buff, "\r\n\r\n") != NULL) break;
    }
    printf("%s\n", message);
    free(message);

    // Send message to server
    while (1)
    {
        printf("Enter string (press enter to escape): ");
        fgets(buff, sizeof(buff), stdin);
        if (strncmp(buff, "\n", 1) == 0) break;
        if (send(clientSocket, buff, strlen(buff), 0) == -1)
        {
            printf("Send Error\n");
            break;
        }
    }

    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}