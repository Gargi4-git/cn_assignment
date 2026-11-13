#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    char ip[50];
    char input[100];
    char response[4096];
    int subnets;
    int received;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(clientSocket, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Enter IP address: ");
    scanf("%49s", ip);

    printf("Enter number of subnets: ");
    scanf("%d", &subnets);

    snprintf(input, sizeof(input), "%s %d", ip, subnets);

    send(clientSocket, input, strlen(input), 0);

    printf("\nWaiting for server response...\n");

    while ((received = recv(clientSocket, response,
                            sizeof(response) - 1, 0)) > 0)
    {
        response[received] = '\0';
        printf("%s", response);
    }

    close(clientSocket);

    return 0;
}