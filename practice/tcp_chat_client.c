#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + id address + port
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
    printf("Connected to server\n");

    // Chatting
    fd_set fdread;
    char buff[512];
    int ret;

    while (1)
    {
        FD_ZERO(&fdread);
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(clientSocket, &fdread);

        ret = select(clientSocket + 2, &fdread, NULL, NULL, NULL);

        if (ret <= 0)
        {
            printf("Select Failed\n");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &fdread))
        {
            fgets(buff, sizeof(buff), stdin);
            buff[strlen(buff) - 1] = 0;
            send(clientSocket, buff, strlen(buff), 0);
        }

        if (FD_ISSET(clientSocket, &fdread))
        {
            ret = recv(clientSocket, buff, sizeof(buff), 0);
            if (ret <= 0) break;    // Disconnected
            buff[ret] = 0;
            printf("%s\n", buff);
        }
    }

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}