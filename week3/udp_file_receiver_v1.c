#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TIME_OUT 9

int main(int argc, char* argv[])    
{
    // Check enough arguments
    // Arguments contain run_file_name, port_number, write_file_name 
    if (argc != 3)
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

    // Set timeout for socket
    struct timeval tv;
    tv.tv_sec = TIME_OUT;
    tv.tv_usec = 0;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
        exit(1);
    }

    // Bind socket to ip address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Bind Failed\n");
        exit(1);
    }

    printf("Waiting for message ...\n");
    
    // Receive message from client
    char buff[512];
    struct sockaddr_in clientAddr;
    int size = sizeof(clientAddr);

    FILE *f = fopen(argv[2], "a");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    int ret;
    while (1)
    {
        ret = recvfrom(serverSocket, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &size);
        if (ret <= 0)
            break;

        // Reset the timeout
        if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("Error");
            break;
        }

        fwrite(buff, 1, ret, f);
        printf("Receive %d bytes message from: %s:%d\n", ret, inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
    }
    fclose(f);

    // Close socket
    close(serverSocket);
    printf("Time out!\n");
    printf("Socket closed\n");
    return 0;
}