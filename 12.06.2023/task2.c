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
#include <pthread.h>

#define MAX_USER 64

int clients[MAX_USER];
int numberOfClients = 0;
pthread_mutex_t numberOfClients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *threadProcessing(void *arg)
{

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
    
        if (numberOfClients < MAX_USER)
        {
            // Mutex
            pthread_mutex_lock(&numberOfClients_mutex);
            clients[numberOfClients] = clientSocket;
            numberOfClients++;
            pthread_mutex_unlock(&numberOfClients_mutex);
            // End mutex

            pthread_create(&thread_id, NULL, threadProcessing, (void *)&clientSocket);
            pthread_detach(thread_id);
        }
        else
        {
            // Full connection
            printf("Exceed maximum connection\n");
            char *message = "Excees maximum connection\n";
            send(clientSocket, message, strlen(message), 0);
            close(clientSocket);
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}