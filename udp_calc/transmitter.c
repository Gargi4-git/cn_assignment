#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int clientsocket;
    int a, b, choice;
    double result;
    char op;
    struct sockaddr_in serveraddress;
    socklen_t serverlen = sizeof(serveraddress);

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (clientsocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serveraddress, 0, sizeof(serveraddress));

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1)
    {
        printf("\nEnter first number: ");
        scanf("%d", &a);

        printf("Enter second number: ");
        scanf("%d", &b);

        printf("Enter operator (+, -, *, /): ");
        scanf(" %c", &op);

        sendto(clientsocket, &a, sizeof(a), 0,
               (struct sockaddr *)&serveraddress, serverlen);

        sendto(clientsocket, &b, sizeof(b), 0,
               (struct sockaddr *)&serveraddress, serverlen);

        sendto(clientsocket, &op, sizeof(op), 0,
               (struct sockaddr *)&serveraddress, serverlen);

        recvfrom(clientsocket, &result, sizeof(result), 0,
                 NULL, NULL);

        if (op == '/' && b == 0)
            printf("Cannot divide by zero!\n");
        else
            printf("Result = %.2f\n", result);

        printf("\nDo you want to continue? (1 = Yes, 0 = No): ");
        scanf("%d", &choice);

        sendto(clientsocket, &choice, sizeof(choice), 0,
               (struct sockaddr *)&serveraddress, serverlen);

        if (choice == 0)
            break;
    }

    close(clientsocket);

    return 0;
}