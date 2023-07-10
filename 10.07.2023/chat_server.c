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
#include <ctype.h>

#define MAX_USER 64

int host = 0;
int user_numbers = 0;
int client[MAX_USER];
char* client_name[MAX_USER];
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

bool join(bool isLogged, char *nickname, int socket)
{
    if (isLogged)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    if (nickname == NULL || strlen(nickname) == 0)
    {
        char *error = "201 INVALID NICK NAME\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    for (int i = 0; i < strlen(nickname); ++i) 
    {
        if (!islower(nickname[i]) && !isdigit(nickname[i]))
        {
            char *error = "201 INVALID NICK NAME\r\n";
            send(socket, error, strlen(error), 0);
            return false;
        }
    }

    pthread_mutex_lock(&user_mutex);
    for (int i = 0; i < user_numbers; ++i)
    {
        if (client_name[i] == NULL) 
            continue;
        if (strcmp(client_name[i], nickname) == 0)
        {
            char *error = "200 NICKNAME IN USE\r\n";
            send(socket, error, strlen(error), 0);
            pthread_mutex_unlock(&user_mutex);
            return false;
        }
    }
    pthread_mutex_unlock(&user_mutex);

    // Adding
    pthread_mutex_lock(&user_mutex);
    int index = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket == client[i])
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    char mess[256];
    sprintf(mess, "JOIN %s\r\n", nickname);
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket != client[i] && client_name[i] != NULL)
        {
            send(client[i], mess, strlen(mess), 0);
        }
    }

    client_name[index] = malloc(strlen(nickname) + 1);
    strcpy(client_name[index], nickname);
    client_name[strlen(nickname)] = 0;

    pthread_mutex_unlock(&user_mutex);
    char *error = "100 OK\r\n";
    send(socket, error, strlen(error), 0);
    return true;
}

bool msg(bool isLogged, char *value, int socket)
{
    if (!isLogged || strlen(value) == 0)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    pthread_mutex_lock(&user_mutex);
    int index = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket == client[i])
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }
    char buff[256];
    sprintf(buff, "MSG %s %s\r\n", client_name[index], value);
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket != client[i] && client_name[i] != NULL)
        {
            send(client[i], buff, strlen(buff), 0);
        }
    }
    char *error = "100 OK\r\n";
    send(socket, error, strlen(error), 0);
    pthread_mutex_unlock(&user_mutex);
    return true;
}

bool pmsg(bool isLogged, char *value, int socket)
{
    if (!isLogged || strlen(value) == 0)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    char sendto[128];
    sscanf(value, "%s", sendto);
    char *mess = value + strlen(sendto) + 1;

    pthread_mutex_lock(&user_mutex);
    int index = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket == client[i])
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    int index_sendto = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (client_name[i] == NULL)
            continue;

        if (strcmp(sendto, client_name[i]) == 0)
        {
            index_sendto = i;
            break;
        }
    }

    if (index_sendto == -1)
    {
        char *error = "202 UNKNOWN NICKNAME\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    char buff[256];
    sprintf(buff, "PMSG %s %s\r\n", client_name[index], mess);
    send(client[index_sendto], buff, strlen(buff), 0);
    
    char *error = "100 OK\r\n";
    send(socket, error, strlen(error), 0);
    pthread_mutex_unlock(&user_mutex);
    return true;
}

bool op(bool isLogged, char *value, int socket)
{
    if (!isLogged || strlen(value) == 0)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    pthread_mutex_lock(&user_mutex);
    int index = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket == client[i])
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    if (index != host)
    {
        char *error = "203 DENIED\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    int index_sendto = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (client_name[i] == NULL)
            continue;

        if (strcmp(value, client_name[i]) == 0)
        {
            index_sendto = i;
            break;
        }
    }

    if (index_sendto == -1)
    {
        char *error = "202 UNKNOWN NICKNAME\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    char mess[256];
    sprintf(mess, "OP %s", client_name[index_sendto]);
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket != client[i] && client_name[i] != NULL)
        {
            send(client[i], mess, strlen(mess), 0);
        }
    }

    host = index_sendto;
    pthread_mutex_unlock(&user_mutex);
    
    char *error = "100 OK\r\n";
    send(socket, error, strlen(error), 0);
    return true;
}

bool kick(bool isLogged, char *value, int socket)
{
    if (!isLogged || strlen(value) == 0)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        return false;
    }

    pthread_mutex_lock(&user_mutex);
    int index = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket == client[i])
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        char *error = "999 UNKNOWN ERROR\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    if (index != host)
    {
        char *error = "203 DENIED\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    int index_sendto = -1;
    for (int i = 0; i < user_numbers; ++i)
    {
        if (client_name[i] == NULL)
            continue;

        if (strcmp(value, client_name[i]) == 0)
        {
            index_sendto = i;
            break;
        }
    }

    if (index_sendto == -1)
    {
        char *error = "202 UNKNOWN NICKNAME\r\n";
        send(socket, error, strlen(error), 0);
        pthread_mutex_unlock(&user_mutex);
        return false;
    }

    char mess[256];
    sprintf(mess, "KICK %s %s", client_name[index_sendto], client_name[index]);
    for (int i = 0; i < user_numbers; ++i)
    {
        if (socket != client[i] && client_name[i] != NULL)
        {
            send(client[i], mess, strlen(mess), 0);
        }
    }

    // Remove from arr
    
    pthread_mutex_unlock(&user_mutex);
    
    char *error = "100 OK\r\n";
    send(socket, error, strlen(error), 0);
    return true;
}

void *threadProcessing(void *arg)
{
    int socket = *(int *)arg;
    bool isLogged = false;
    char buff[1024];
    char cmd[32];
    char value[512];

    while (1)
    {
        int ret = recv(socket, buff, sizeof(buff) - 1, 0);
        if (ret <= 0)
            break;
        buff[ret] = 0;
        if (buff[ret - 1] = '\n')
            buff[ret - 1] = 0;

        sscanf(buff, "%s", cmd);
        int cmd_len = strlen(cmd) + 1;
        strcpy(value, buff + cmd_len);
        printf("%s\n", cmd);
        printf("%s %ld\n", value, strlen(value));
        value[strlen(value)] = 0;
        if (strcmp(cmd, "JOIN") == 0)
        {
            // JOIN
            if (join(isLogged, value, socket))
                isLogged = true;
        }
        else if (strcmp(cmd, "MSG") == 0)
        {
            // MSG
            msg(isLogged, value, socket);
        }
        else if (strcmp(cmd, "PMSG") == 0)
        {
            // PMSG
            pmsg(isLogged, value, socket);
        }
        else if (strcmp(cmd, "OP") == 0)
        {
            // OP
            op(isLogged, value, socket);
        }
        else if (strcmp(cmd, "KICK") == 0)
        {
            // KICK
            kick(isLogged, value, socket);
        }
        else if (strcmp(cmd, "TOPIC") == 0)
        {
            // TOPIC
        }
        else if (strcmp(cmd, "QUIT") == 0)
        {
            // QUIT
        }
        else
        {
            char *error = "999 UNKNOWN ERROR\r\n";
            send(socket, error, strlen(error), 0);
        }
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

    // Write code here
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
            pthread_detach(thread_id);

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