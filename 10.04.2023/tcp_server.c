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
    if (argc != 4)
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

    // Send hello message to client
    char *filename = argv[2];
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    char buf[512];
    int ret;
    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        if (send(clientSocket, buf, ret, 0) == -1)
        {
            printf("Send hello to client failed\n");
            exit(1);
        }
    }
    send(clientSocket, "\r\n\r\n", 4, 0);   // send close signal to client
    fclose(f);

    // Write client's message to file
    filename = argv[3];
    f = fopen(filename, "a");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    while (1)
    {
        int ret = recv(clientSocket, buf, sizeof(buf), 0);
        if (ret <= 0) break;
        fwrite(buf, 1, ret, f);
        printf("Writed\n");
    }
    fclose(f);

    // Close
    close(clientSocket);
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}