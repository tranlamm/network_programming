#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1)
    {
        printf("Create socket failed");
        return 1;
    }

    // Declare struct of ip address server 
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9000);

    // Connect server
    int ret = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        printf("Connect Failed");
        return 1;
    }

    // Receive message
    char buf[256];
    ret = recv(client_socket, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("Receive Failed");
        return 1;
    }
    else
    {
        printf("Receive %d bytes\n", ret);
        if (ret < sizeof(buf))
            buf[ret] = 0;
        printf("Message: %s\n", buf);
    }

    // Send Message
    char buffer[512];
    while (1)
    {
        printf("Enter message: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (strncmp(buffer, "exit", 4) == 0) break;

        int ret = send(client_socket, buffer, strlen(buffer), 0);
        if (ret == -1) break;
    }
    
    // Close
    close(client_socket);
    return 0;
}