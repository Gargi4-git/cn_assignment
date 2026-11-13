#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    int clientSocket;
    char serverResponse[256];
    char message[] = "Hello Server";

    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(clientSocket < 0)
    {
        printf("Socket is not created\n");
        return 1;
    }
    else
    {
        printf("Socket created successfully\n");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(clientSocket,
           message,
           sizeof(message),
           0,
           (struct sockaddr *)&serverAddress,
           sizeof(serverAddress));

    socklen_t len = sizeof(serverAddress);

    recvfrom(clientSocket,
             serverResponse,
             sizeof(serverResponse),
             0,
             (struct sockaddr *)&serverAddress,
             &len);

    printf("Reply from server: %s\n", serverResponse);

    close(clientSocket);

    return 0;
}