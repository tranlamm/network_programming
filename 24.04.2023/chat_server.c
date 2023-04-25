#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

struct Client_Info 
{
    int fd;
    char client_id[128];    // client_name
};

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Arguments contains ./run_file_name + port
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

    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    time_t t;
    t = time(NULL);
    struct tm tm = *localtime(&t);

    fd_set fdread;
    struct Client_Info clients[64];
    int numberOfClients = 0;
    char buff[512];
    char sendBuff[1024];
    int maxFd, ret;
    while (1)
    {
        FD_ZERO(&fdread);

        // Add fd to fdread
        FD_SET(serverSocket, &fdread);
        maxFd = serverSocket + 1;
        for (int i = 0; i < numberOfClients; ++i)
        {
            int fd = clients[i].fd;
            FD_SET(fd, &fdread);
            
            // find maximum socket description
            if (fd + 1 > maxFd) 
                maxFd = fd + 1;
        }

        if (select(maxFd + 1, &fdread, NULL, NULL, NULL) <= 0)
        {
            printf("Select Failed\n");
            exit(1);
        }

        if (FD_ISSET(serverSocket, &fdread))
        {
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
            printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // Add client_info
            struct Client_Info client_info;
            client_info.fd = clientSocket;
            client_info.client_id[0] = 0;
            clients[numberOfClients] = client_info;
            numberOfClients++;

            // Ask for client name
            send(clientSocket, "Please enter your name (client_id:client_name)", 46, 0);
        }

        for (int i = 0; i < numberOfClients; ++i)
        {
            int fd = clients[i].fd;
            if (FD_ISSET(fd, &fdread))
            {
                ret = recv(fd, buff, sizeof(buff), 0);
                if (ret <= 0)
                {
                    // Client disconnected
                    printf("Client %d disconnected\n", fd);
                    close(fd);

                    // Delete client
                    for (int k = i; k < numberOfClients - 1; k++)
                    {
                        clients[k] = clients[k + 1];
                    }
                    numberOfClients--;
                    i--;
                    continue;
                }

                if (strcmp(clients[i].client_id, "") == 0)  // Client hasn't had a name
                {
                    if (strncmp(buff, "client_id:", 10) == 0)
                    {
                        // Right format "client_id:client_name"
                        memcpy(clients[i].client_id, buff + 10, ret - 10);
                    }
                    else
                    {
                        // Wrong format
                        send(fd, "Please enter your name (client_id:client_name)", 46, 0);
                    }
                }
                else    // Client has already had a name
                {
                    sprintf(sendBuff, "%04d/%02d/%02d %02d:%02d:%02d %s:%s", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
                    tm.tm_hour, tm.tm_min, tm.tm_sec, clients[i].client_id, buff);
                    int length = strlen(clients[i].client_id) + ret + 21;
                    for (int j = 0; j < numberOfClients; ++j)
                    {
                        // Boardcast message to other clients
                        if (i != j)
                            send(clients[j].fd, sendBuff, length, 0);
                    }
                }
            }
        }
    }   

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}