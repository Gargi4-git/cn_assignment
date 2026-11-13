#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 26

int main()
{
    int sock, n, edges;
    char buffer[4096];
    int pos = 0;

    struct sockaddr_in server;

    printf("Enter number of nodes: ");

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX)
    {
        printf("Invalid number of nodes.\n");
        return 1;
    }

    printf("Nodes: ");

    for (int i = 0; i < n; i++)
        printf("%c ", 'A' + i);

    printf("\nEnter number of edges: ");

    if (scanf("%d", &edges) != 1 ||
        edges < 0 || edges > n * (n - 1) / 2)
    {
        printf("Invalid number of edges.\n");
        return 1;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "%d %d\n", n, edges);

    printf("\nEnter edges (Node1 Node2 Weight):\n");

    for (int i = 0; i < edges; i++)
    {
        char u, v;
        int w;

        if (scanf(" %c %c %d", &u, &v, &w) != 3 ||
            u < 'A' || u >= 'A' + n ||
            v < 'A' || v >= 'A' + n ||
            u == v || w < 0)
        {
            printf("Invalid edge. Try again with valid input.\n");
            return 1;
        }

        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                        "%c %c %d\n", u, v, w);
    }

    char source;

    printf("Enter source node: ");

    if (scanf(" %c", &source) != 1 ||
        source < 'A' || source >= 'A' + n)
    {
        printf("Invalid source node.\n");
        return 1;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "%c\n", source);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send the entire message.
    int sent = 0;

    while (sent < pos)
    {
        int bytes = send(sock, buffer + sent, pos - sent, 0);

        if (bytes <= 0)
        {
            perror("Send failed");
            close(sock);
            return 1;
        }

        sent += bytes;
    }

    // Tell the server that no more data will be sent.
    shutdown(sock, SHUT_WR);

    printf("\nGraph sent to server successfully.\n");

    close(sock);
    return 0;
}
