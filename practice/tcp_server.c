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
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        perror("Create Socket Failed: ");
        return 1;
    }

    // Create struct of ip address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Bind socket with ip address
    int ret = bind(server_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf("Bind Failed: \n");
        return 1;
    }

    // Listen
    ret = listen(server_socket, 5);
    if (ret == -1)
    {
        printf("Listen Failed: \n");
        return 1;
    }
    printf("Waiting for new client ...\n");

    // Accept
    struct sockaddr_in client_addr;
    int client_addr_length = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_length);
    if (client_socket == -1)
    {
        printf("Listen Failed: \n");
        return 1;
    }
    printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

    // Send 
    char *buf = "Welcome client";
    ret = send(client_socket, buf, strlen(buf), 0);
    if (ret == -1) {
        printf("Send Failed");
        return 1;
    }
    
    // Receive 
    char buffer[256];
    while (1)
    {
        int ret = recv(client_socket, buffer, sizeof(buffer), 0);
        if (ret <= 0) break;
        buffer[ret] = 0;
        printf("%s", buffer);
    }

    // Close
    close(client_socket);
    close(server_socket);
    return 0;
}