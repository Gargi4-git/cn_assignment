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
    int serverSocket;
    char clientMessage[256];
    char serverReply[] = "Hello Client";

    struct sockaddr_in serverAddress, clientAddress;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }
    else
    {
        printf("Socket created successfully\n");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket,
         (struct sockaddr *)&serverAddress,
         sizeof(serverAddress));

    printf("Server is waiting for client...\n");

    socklen_t len = sizeof(clientAddress);

    recvfrom(serverSocket,
             clientMessage,
             sizeof(clientMessage),
             0,
             (struct sockaddr *)&clientAddress,
             &len);

    printf("Message from client: %s\n", clientMessage);

    sendto(serverSocket,
           serverReply,
           sizeof(serverReply),
           0,
           (struct sockaddr *)&clientAddress,
           len);

    printf("Reply sent to client.\n");

    close(serverSocket);

    return 0;
}