
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serversocket, client_socket;
    int a, b, choice;
    double result;
    char op;
    struct sockaddr_in serveraddress;

    serversocket = socket(AF_INET, SOCK_STREAM, 0);

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    bind(serversocket, (struct sockaddr *)&serveraddress,
         sizeof(serveraddress));

    listen(serversocket, 3);

    printf("Server waiting for client...\n");

    client_socket = accept(serversocket, NULL, NULL);

    while (1)
    {
        recv(client_socket, &a, sizeof(a), 0);
        recv(client_socket, &b, sizeof(b), 0);
        recv(client_socket, &op, sizeof(op), 0);

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

        send(client_socket, &result, sizeof(result), 0);

        recv(client_socket, &choice, sizeof(choice), 0);

        if (choice == 0)
            break;
    }

    close(client_socket);
    close(serversocket);

    return 0;
}

