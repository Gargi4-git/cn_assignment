#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sock;
    struct sockaddr_in server;
    char ip[100];
    int i = 0, ch;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("Socket failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        close(sock);
        return 1;
    }

    printf("Enter IP Address: ");

    while ((ch = getchar()) != '\n' && ch != EOF && i < 99)
    {
        ip[i] = ch;
        i++;
    }

    ip[i] = '\0';

    send(sock, ip, i, 0);

    close(sock);

    return 0;
}
