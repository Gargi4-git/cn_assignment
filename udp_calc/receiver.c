#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int serversocket;
    int a, b, choice;
    double result;
    char op;
    struct sockaddr_in serveraddress, clientaddress;
    socklen_t clientlen = sizeof(clientaddress);

    serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (serversocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serveraddress, 0, sizeof(serveraddress));

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serversocket, (struct sockaddr *)&serveraddress,
             sizeof(serveraddress)) < 0)
    {
        perror("Bind failed");
        close(serversocket);
        return 1;
    }

    printf("UDP Server waiting for client...\n");

    while (1)
    {
        recvfrom(serversocket, &a, sizeof(a), 0,
                 (struct sockaddr *)&clientaddress, &clientlen);

        recvfrom(serversocket, &b, sizeof(b), 0,
                 (struct sockaddr *)&clientaddress, &clientlen);

        recvfrom(serversocket, &op, sizeof(op), 0,
                 (struct sockaddr *)&clientaddress, &clientlen);

        switch (op)
        {
            case '+':
                result = a + b;
                break;

            case '-':
                result = a - b;
                break;

            case '*':
                result = a * b;
                break;

            case '/':
                if (b != 0)
                    result = (double)a / b;
                else
                    result = 0;
                break;

            default:
                result = 0;
        }

        if (op == '/' && b == 0)
            printf("Cannot divide by zero!\n");
        else
            printf("Result = %.2f\n", result);

        sendto(serversocket, &result, sizeof(result), 0,
               (struct sockaddr *)&clientaddress, clientlen);

        recvfrom(serversocket, &choice, sizeof(choice), 0,
                 (struct sockaddr *)&clientaddress, &clientlen);

        if (choice == 0)
            break;
    }

    close(serversocket);

    return 0;
}