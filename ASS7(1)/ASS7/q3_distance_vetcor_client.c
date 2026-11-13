#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 10
#define MAXE 45

struct Edge
{
    int u, v, cost;
};

struct Graph
{
    int n, m;
    struct Edge e[MAXE];
    int source, destination;
};

int main()
{
    int sock, i;
    struct sockaddr_in server;
    struct Graph g;

    char u, v, s, d;

    printf("Enter number of nodes: ");
    scanf("%d", &g.n);

    printf("Enter number of edges: ");
    scanf("%d", &g.m);

    if (g.n < 1 || g.n > MAX ||
        g.m < 0 || g.m > MAXE)
    {
        printf("Invalid input.\n");
        return 1;
    }

    printf("\nEnter edges (Node1 Node2 Cost):\n");

    for (i = 0; i < g.m; i++)
    {
        scanf(" %c %c %d",
              &u, &v, &g.e[i].cost);

        g.e[i].u = u - 'A';
        g.e[i].v = v - 'A';
    }

    printf("Enter source node: ");
    scanf(" %c", &s);

    printf("Enter destination node: ");
    scanf(" %c", &d);

    g.source = s - 'A';
    g.destination = d - 'A';

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        printf("Connection failed. Start the server first.\n");
        close(sock);
        return 1;
    }

    /* Send the graph to the server */
    char *p = (char *)&g;
    int total = 0;

    while (total < sizeof(g))
    {
        int sent = send(sock, p + total,
                        sizeof(g) - total, 0);

        if (sent <= 0)
        {
            printf("Sending failed.\n");
            close(sock);
            return 1;
        }

        total = total + sent;
    }

    printf("\nGraph sent to server successfully.\n");

    close(sock);

    return 0;
}
