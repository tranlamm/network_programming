#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    fd_set fdread, fdtest;
    
    // Xóa tất cả socket trong tập fdread
    FD_ZERO(&fdread);
    
    // Thêm socket listener vào tập fdread
    FD_SET(listener, &fdread);

    char buf[256];
    struct timeval tv;
    
    int users[64];      // Mang socket client da dang nhap
    char *user_ids[64]; // Mang id client da dang nhap
    int num_users = 0;  // So luong client da dang nhap

    while (1)
    {
        fdtest = fdread;

        // Thiet lap thoi gian cho
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Chờ đến khi sự kiện xảy ra
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        if (ret == 0)
        {
            printf("Timed out!!!\n");
            continue;
        }

        for (int i = listener; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    int client = accept(listener, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        FD_SET(client, &fdread);
                        printf("New client connected: %d\n", client);
                    }
                    else
                    {
                        // Dang co qua nhieu ket noi
                        close(client);
                    }
                }
                else
                {
                    int ret = recv(i, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        // Delete socket in users array
                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == i)
                                break;

                        users[j] = users[num_users - 1];
                        strcpy(user_ids[j], user_ids[num_users - 1]);
                        num_users--;
                        FD_CLR(i, &fdread);
                        close(i);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", i, buf);

                        int client = i;

                        // Kiem tra trang thai dang nhap
                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client)
                                break;
                        
                        if (j == num_users) // Chua dang nhap
                        {
                            // Xu ly cu phap lenh dang nhap
                            char cmd[32], id[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                            if (ret == 2)
                            {
                                if (strcmp(cmd, "client_id:") == 0)
                                {
                                    char *msg = "Dung cu phap. Hay nhap tin nhan de chuyen tiep.\n";
                                    send(client, msg, strlen(msg), 0);

                                    // Luu vao mang user
                                    users[num_users] = client;
                                    user_ids[num_users] = malloc(strlen(id) + 1);
                                    strcpy(user_ids[num_users], id);
                                    num_users++;
                                }
                                else
                                {
                                    char *msg = "Sai cu phap. Hay nhap lai.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                            }
                            else
                            {
                                char *msg = "Sai tham so. Hay nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                        }
                        else // Da dang nhap
                        {
                            // id: user_ids[j]
                            // data: buf

                            char sendbuf[256];

                            strcpy(sendbuf, user_ids[j]);
                            strcat(sendbuf, ": ");
                            strcat(sendbuf, buf);

                            // Forward du lieu cho cac user
                            for (int k = 0; k < num_users; k++)
                                if (users[k] != client)
                                    send(users[k], sendbuf, strlen(sendbuf), 0);
                        }
                    }
                }
            }
        }
    }

    close(listener);    

    return 0;
}