#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

struct SinhVien {
    char mssv[9];
    char hoTen[64];
    char ngaySinh[11];
    float diemTrungBinh;
};

int main(int argc, char* argv[])
{
    // Check enough arguments
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
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
    if (clientSocket == -1)
    {
        printf("Accept connection failed\n");
        exit(1);
    }
    printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
    // Write client's message to file
    char *filename = argv[2];
    FILE *f = fopen(filename, "a");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    int ret;
    time_t t;
    t = time(NULL);
    struct tm tm = *localtime(&t);
    char buff[512];
    while (1)
    {
        struct SinhVien sv;    
        ret = recv(clientSocket, sv.mssv, sizeof(sv.mssv), 0);
        if (ret <= 0) break;

        ret = recv(clientSocket, sv.hoTen, sizeof(sv.hoTen), 0);
        if (ret <= 0) break;

        ret = recv(clientSocket, sv.ngaySinh, sizeof(sv.ngaySinh), 0);
        if (ret <= 0) break;

        ret = recv(clientSocket, &sv.diemTrungBinh, sizeof(sv.diemTrungBinh), 0);
        if (ret <= 0) break;

        sprintf(buff, "%s:%d %d-%d-%d %d:%d:%d %s %s %s %.2f\n", 
        inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, sv.mssv, sv.hoTen, sv.ngaySinh, sv.diemTrungBinh);

        printf("%s", buff);
        fwrite(buff, 1, strlen(buff), f);
    }
    fclose(f);

    // Close
    close(clientSocket);
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}