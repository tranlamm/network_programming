#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/select.h>

#define MAX_USER 64

char* chuanHoaXau(char* xau) {
    int i, j = 0;
    int length = strlen(xau);
    
    // Loại bỏ khoảng trắng thừa ở đầu xâu
    while (xau[j] == ' ') {
        j++;
    }
    if (j > 0) {
        for (i = 0; i < length - j; i++) {
            xau[i] = xau[i + j];
        }
        xau[i] = '\0';
        length -= j;
    }
    
    // Loại bỏ khoảng trắng thừa ở cuối xâu
    i = length - 1;
    while (xau[i] == ' ') {
        xau[i] = '\0';
        i--;
    }
    
    // Chuẩn hóa khoảng trắng giữa các từ
    for (i = 0; i < length; i++) {
        if (xau[i] == ' ' && xau[i + 1] == ' ') {
            int j = i;
            while (xau[j] != '\0') {
                xau[j] = xau[j + 1];
                j++;
            }
            i--;
            length--;
        }
    }
    
    // Viết hoa chữ cái đầu mỗi từ
    for (i = 0; i < length; i++) {
        if (i == 0 || xau[i - 1] == ' ') {
            if (xau[i] >= 'a' && xau[i] <= 'z') {
                xau[i] = xau[i] - 32;
            }
        } else {
            if (xau[i] >= 'A' && xau[i] <= 'Z') {
                xau[i] = xau[i] + 32;
            }
        }
    }
    return xau;
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

    // Variable for logging client address
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);

    // Declare and add socket to Fdset
    fd_set fdread, fdclone;
    FD_ZERO(&fdread);
    FD_SET(serverSocket, &fdread); 

    // Listening
    char buff[256];
    int ret = 0;
    int numUsers = 0;

    while (1)
    {
        fdclone = fdread;
     
        ret = select(FD_SETSIZE, &fdclone, NULL, NULL, NULL);
        if (ret <= 0)
        {
            printf("Select Failed\n");
            break;
        }

        // Listen message
        for (int i = serverSocket; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &fdclone))
            {
                // Listen connection
                if (i == serverSocket)
                {
                    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);

                    if (numUsers < MAX_USER)
                    {
                        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                        FD_SET(clientSocket, &fdread);
                        numUsers++;

                        char message[256];
                        sprintf(message, "Xin chao. Hien co %d client dang ket noi.\n", numUsers);
                        send(clientSocket, message, strlen(message), 0);
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
                else
                {
                    // Receive message
                    ret = recv(i, buff, sizeof(buff), 0);
                    buff[ret] = 0;

                    if (ret <= 0)
                    {
                        numUsers--;
                        FD_CLR(i, &fdread);
                        close(i);
                    }

                    char *afterNormalize = chuanHoaXau(buff);
                    if (strncmp(afterNormalize, "Exit", 4) == 0)
                    {
                        char *message = "Chao tam biet\n";
                        send(i, message, strlen(message), 0);
                        numUsers--;
                        FD_CLR(i, &fdread);
                        close(i);
                    }
                    else
                    {
                        send(i, afterNormalize, strlen(afterNormalize), 0);
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