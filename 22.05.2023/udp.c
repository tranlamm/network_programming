#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

int main(int argc, char *argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + ip_address_receiver + port_receiver + port_sender
    if (argc != 4)
    {
        printf("Missing arguments\n");
        exit(1);
    }

    // Declare receiver's socket
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Declare sockaddr of receiver
    struct sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_addr.s_addr = htonl(INADDR_ANY);
    raddr.sin_port = htons(atoi(argv[3]));

    // Declare sender's socket
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Declare sockaddr's of sender
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(argv[1]);
    saddr.sin_port = htons(atoi(argv[2]));

    // Bind receiver socket to sockaddr
    bind(receiver, (struct sockaddr *)&raddr, sizeof(raddr));

    // Declare fd_set
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(receiver, &fdread);

    // Listening
    printf("Chatting ...\n");
    char buff[256];
    while (1)
    {
        fdtest = fdread;
        int ret = select(receiver + 1, &fdtest, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("Select() failed\n");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdtest))
        {
            fgets(buff, sizeof(buff), stdin);
            if (buff[strlen(buff) - 1] == '\n')
                buff[strlen(buff) - 1] = 0;
            sendto(sender, buff, strlen(buff), 0, (struct sockaddr *)&saddr, sizeof(saddr));
        }

        if (FD_ISSET(receiver, &fdtest))
        {
            ret = recvfrom(receiver, buff, sizeof(buff), 0, NULL, NULL);
            if (ret < sizeof(buff))
                buff[ret] = 0;
            printf("Received: %s\n", buff);
        }
    }

    return 0;
}