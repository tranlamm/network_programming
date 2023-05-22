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
    // Arguments contain run_file_name, ip_address, port_number, deliver_file_name
    if (argc != 4)
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

    // Send file
    char buff[512];
    FILE *f = fopen(argv[3], "r");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    int ret;
    while (!feof(f))
    {
        ret = fread(buff, 1, sizeof(buff), f);
        if (ret <= 0) break;

        if (sendto(clientSocket, buff, ret, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) break;
        printf("%d bytes have been send\n", ret);
    }
    fclose(f);

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}