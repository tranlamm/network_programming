#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/select.h>
#include <ctype.h>

char result[256];

void standardized_optimize(char *str)
{
    memset(result, 0, sizeof(result));
    bool isFirst = true;
    int len = strlen(str);
    char tmp;

    // Delete line break
    if (str[len - 1] == '\n')
    {
        str[len - 1] = 0;
        len--;
    }

    // Standardized
    for (int i = 0; i < len; ++i)
    {
        if (str[i] != ' ')
        {
            if (isFirst)
            {
                tmp = toupper(str[i]);
                result[0] = tmp;
                isFirst = false;
            }
            else if (str[i - 1] == ' ')
            {
                tmp = ' ';
                strncat(result, &tmp, 1);
                tmp = toupper(str[i]);
                strncat(result, &tmp, 1);
            }
            else
            {
                tmp = str[i];
                strncat(result, &tmp, 1);
            }
        }
    }
}

char* standardized(char *str)
{
    int len = strlen(str);
    int tmp = 0;

    // Delete line break
    if (str[len - 1] == '\n')
    {
        str[len - 1] = 0;
        len--;
    }

    // Delete all space in head
    for (tmp = 0; tmp < len; ++tmp)
    {
        if (str[tmp] != ' ')
            break;
    }
    len -= tmp;

    for (int i = 0; i < len; ++i)
    {
        str[i] = str[i + tmp];
    }
    str[len] = 0;

    // Delete all space in tail
    for (tmp = len - 1; tmp >= 0; --tmp)
    {
        if (str[tmp] != ' ')
            break;
    }
    len = tmp + 1;
    str[len] = 0;

    // Delete space in middle
    for (int i = 1; i < len; ++i)
    {
        if (str[i] == ' ' && str[i + 1] == ' ')
        {
            for (int j = i; j < len - 1; ++j)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
    str[len] = 0;
    
    // Uppercase
    str[0] = toupper(str[0]);
    for (int i = 0; i < len; ++i)
    {
        if (str[i] == ' ')
            str[i + 1] = toupper(str[i + 1]);
    }

    return str;
}

int main(int argc, char* argv[])    
{
    // Check enough arguments
    // Arguments contain ./run_file_name + ip_recv + port_recv + port_send
    if (argc != 4)
    {
        printf("Not enough arguments\n");
        exit(1);
    }

    // Create socket
    int _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == -1)
    {
        perror("Create socket failed\n");
        exit(1);
    }

    // Declare receiver's sockaddr
    struct sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_addr.s_addr = inet_addr(argv[1]);
    raddr.sin_port = htons(atoi(argv[2]));

    // Declare sender's sockaddr and bind to socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[3]));

    if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("Binding failed\n");
        exit(1);
    }

    // Declare fd_set
    fd_set fdread, fdclone;
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(_socket, &fdread);

    // Listening
    printf("Chatting ...\n");
    struct sockaddr_in log_addr;
    int size = sizeof(log_addr);
    int ret = 0;
    char buff[256];
    while (1)
    {
        fdclone = fdread;

        ret = select(_socket + 1, &fdclone, NULL, NULL, NULL);
        if (ret == -1)
        {
            printf("Select failed\n");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdclone))
        {
            fgets(buff, sizeof(buff), stdin);
            sendto(_socket, buff, strlen(buff), 0, (struct sockaddr*)&raddr, sizeof(raddr));
        }

        if (FD_ISSET(_socket, &fdclone))
        {
            ret = recvfrom(_socket, buff, sizeof(buff), 0, (struct sockaddr*)&log_addr, &size);
            buff[ret] = 0;
            printf("Receive from %s:%d: %s\n", inet_ntoa(log_addr.sin_addr), ntohs(log_addr.sin_port), standardized(buff));
        }
    }

    // Close socket
    close(_socket);
    printf("Socket closed\n");
    return 0;
}   