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
#include <dirent.h>

void signalHandler(int signo)
{
    int stat;
    int pid = wait(&stat);
    if (stat == EXIT_SUCCESS)
        printf("Client %d terminated\n", pid);
}

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + port + directory_path
    if (argc != 3)
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
    
    // Listening
    signal(SIGCHLD, signalHandler);
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
        if (fork() == 0)
        {
            // Child process
            close(serverSocket);
            
            int file_numbers = 0;
            char* file_list = malloc(1);
            file_list[0] = '\0';

            // Dirent to read all file in directory
            DIR *dir = opendir(argv[2]);
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_REG)
                {
                    file_numbers++;
                    file_list = realloc(file_list, strlen(file_list) + strlen(entry->d_name) + 3);
                    strcat(file_list, entry->d_name);
                    strcat(file_list, "\r\n\0");
                }
            }

            if (file_numbers == 0)
            {
                char *msg = "ERROR No files to download\n";
                send(clientSocket, msg, strlen(msg), 0);
                free(file_list);
                close(clientSocket);
                exit(EXIT_SUCCESS);
            }

            // Send list of files
            char buff[512];
            sprintf(buff, "OK %d\r\n", file_numbers);
            send(clientSocket, buff, strlen(buff), 0);
            send(clientSocket, file_list, strlen(file_list), 0);
            send(clientSocket, "\r\n", 2, 0);
            free(file_list);

            while (1)
            {
                int ret = recv(clientSocket, buff, sizeof(buff), 0);
                if (ret <= 0)
                    break;
                buff[ret] = 0;
                if (buff[ret - 1] == '\n')
                    buff[ret - 1] = 0;

                FILE *f = fopen(buff, "rb");
                if (f == NULL)
                {
                    char *msg = "ERROR files is not existed\n";
                    send(clientSocket, msg, strlen(msg), 0);
                }
                else
                {
                    // Get size of file
                    fseek(f, 0, SEEK_END);
                    long file_size = ftell(f);
                    fseek(f, 0, SEEK_SET);

                    sprintf(buff, "OK %ld\r\n", file_size);
                    send(clientSocket, buff, strlen(buff), 0);
                    while (!feof(f))
                    {
                        ret = fread(buff, 1, sizeof(buff), f);
                        if (ret <= 0)
                            break;
                        send(clientSocket, buff, ret, 0);
                    }

                    send(clientSocket, "\r\n\r\n", 4, 0);
                    fclose(f);
                    break;
                }
            }

            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Main process
            close(clientSocket);
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    killpg(0, SIGKILL);
    return 0;
}