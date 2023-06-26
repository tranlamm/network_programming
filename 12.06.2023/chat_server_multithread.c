#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_USER 64

int user_numbers = 0;
int client[MAX_USER];
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

void *threadProcessing(void *arg)
{
    int socket = *(int *)arg;
    bool isLogged = false, isDisconnected = false;
    char buff[256], sendMsg[512];
    char client_name[64];

    // Listening
    while (1)
    {
        int ret = recv(socket, buff, sizeof(buff), 0);
        if (ret <= 0)
            break;
        buff[ret] = 0;
        if (buff[ret - 1] = '\n')
            buff[ret - 1] = 0;

        if (!isLogged)
        {
            char cmd[32], client_id[32], tmp[32];
            ret = sscanf(buff, "%s%s%s", cmd, client_id, tmp);
            if (ret != 2)
            {
                // Wrong format
                char *message = "Wrong format (client_id: client_name)\n";
                send(socket, message, strlen(message), 0);
                continue;
            }

            if (strcmp(cmd, "client_id:") != 0)
            {
                // Wrong format
                char *message = "Wrong format (client_id: client_name)\n";
                send(socket, message, strlen(message), 0);
                continue;
            }

            // Right format "client_id: client_name"
            strcpy(client_name, client_id);
            char *message = "Login successfully\n";
            send(socket, message, strlen(message), 0);
            isLogged = true;
        }      
        else
        {
            pthread_mutex_lock(&user_mutex);
            int idx = -1;
            for (int i = 0; i < user_numbers; ++i)
            {
                if (client[i] == socket)
                {
                    idx = i;
                    break;
                }
            }

            if (idx == -1)
            {
                pthread_mutex_unlock(&user_mutex);
                isDisconnected = true;
            }

            sprintf(sendMsg, "%s: %s\n", client_name, buff);
            if (idx % 2 == 0)
            {
                if (idx == user_numbers - 1)
                {
                    char *message = "No user in chat room\n";
                    send(socket, message, strlen(message), 0);
                    pthread_mutex_unlock(&user_mutex);
                    continue;
                }
                send(client[idx + 1], sendMsg, strlen(sendMsg), 0);
            }
            else
            {
                send(client[idx - 1], sendMsg, strlen(sendMsg), 0);
            }
            pthread_mutex_unlock(&user_mutex);
        }  
    }

    // Disconnect
    if (!isDisconnected)
    {
        pthread_mutex_lock(&user_mutex);
        int idx;
        for (int i = 0; i < user_numbers; ++i)
        {
            if (client[i] == socket)
            {
                idx = i;
                break;
            }
        }
        sprintf(sendMsg, "%s disconnected\n", client_name);
        if (idx % 2 == 1)
        {
            send(client[idx - 1], sendMsg, strlen(sendMsg), 0);
            idx--;
        }
        else
        {
            send(client[idx + 1], sendMsg, strlen(sendMsg), 0);
        }
        close(client[idx]);
        close(client[idx + 1]);
        printf("Client %d disconnected\n", client[idx]);
        printf("Client %d disconnected\n", client[idx + 1]);
        for (int i = idx; i < user_numbers - 2; ++i)
        {
            client[i] = client[i + 2];
        }
        user_numbers = user_numbers - 2;
        pthread_mutex_unlock(&user_mutex);
    }
}

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + port
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
    pthread_t thread_id;
    
    // Listening
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if (user_numbers < MAX_USER)
        {
            pthread_mutex_lock(&user_mutex);
            
            client[user_numbers] = clientSocket;
            user_numbers++;
            pthread_create(&thread_id, NULL, threadProcessing, (void *)&clientSocket);

            pthread_mutex_unlock(&user_mutex);
        }
        else
        {
            // Connection full
            char *msg = "Exceed maximum user connection\n";
            send(clientSocket, msg, strlen(msg), 0);
            close(clientSocket);
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}