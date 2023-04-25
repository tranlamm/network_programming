#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

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
    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Connect to server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    
    if (connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Connecting failed\n");
        exit(1);
    }

    // Send message to server
    struct SinhVien sv;
    char tmp[4];
    while (1)
    {
        printf("Nhap mssv: ");
        scanf("%s", sv.mssv);

        printf("Nhap ho va ten: ");
        while (getchar() != '\n');
        fgets(sv.hoTen, sizeof(sv.hoTen), stdin);
        if (sv.hoTen[strlen(sv.hoTen) - 1] == '\n')
            sv.hoTen[strlen(sv.hoTen) -1] = 0;

        printf("Nhap ngay sinh: ");
        scanf("%s", sv.ngaySinh);

        printf("Nhap diem trung binh: ");
        scanf("%f", &sv.diemTrungBinh);
        
        if (send(clientSocket, &sv, sizeof(sv), 0) == -1)
        {
            printf("Send Error\n");
            break;
        }
        
        printf("Ban co muon tiep tuc (yes/no): ");
        scanf("%s", tmp);
        if (strncmp(tmp, "no", 2) == 0) break;
    }

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}