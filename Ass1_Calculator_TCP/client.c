
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(clientsocket, (struct sockaddr *)&serveraddress,
            sizeof(serveraddress));

    while (1)
    {
        printf("\nEnter first number: ");
        scanf("%d", &a);

        printf("Enter second number: ");
        scanf("%d", &b);

        printf("Enter operator (+, -, *, /): ");
        scanf(" %c", &op);

        send(clientsocket, &a, sizeof(a), 0);
        send(clientsocket, &b, sizeof(b), 0);
        send(clientsocket, &op, sizeof(op), 0);

        recv(clientsocket, &result, sizeof(result), 0);

        if (op == '/' && b == 0)
            printf("Cannot divide by zero!\n");
        else
            printf("Result = %.2f\n", result);

        printf("\nDo you want to continue? (1 = Yes, 0 = No): ");
        scanf("%d", &choice);

        send(clientsocket, &choice, sizeof(choice), 0);

        if (choice == 0)
            break;
    }

    close(clientsocket);

    return 0;
}

