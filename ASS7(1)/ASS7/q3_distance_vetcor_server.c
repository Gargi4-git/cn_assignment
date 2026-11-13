#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 10
#define MAXE 45
#define INF 999999

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
    int server_fd, client_fd;
    struct sockaddr_in server;

    struct Graph g;

    int dist[MAX];
    int parent[MAX];
    int i, j, pass;
    int u, v, cost, changed;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed.\n");
        close(server_fd);
        return 1;
    }

    listen(server_fd, 5);

    printf("Server waiting for client...\n");

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0)
    {
        printf("Connection failed.\n");
        close(server_fd);
        return 1;
    }

    /* Receive the complete graph */
    char *p = (char *)&g;
    int total = 0;

    while (total < sizeof(g))
    {
        int received = recv(client_fd, p + total,
                            sizeof(g) - total, 0);

        if (received <= 0)
        {
            printf("Receiving failed.\n");
            close(client_fd);
            close(server_fd);
            return 1;
        }

        total = total + received;
    }

    close(client_fd);

    /* Initialize distances */
    for (i = 0; i < g.n; i++)
    {
        dist[i] = INF;
        parent[i] = -1;
    }

    dist[g.source] = 0;

    /* Calculate shortest distances */
    for (pass = 1; pass < g.n; pass++)
    {
        changed = 0;

        for (j = 0; j < g.m; j++)
        {
            u = g.e[j].u;
            v = g.e[j].v;
            cost = g.e[j].cost;

            /* Check u to v */
            if (dist[u] != INF &&
                dist[u] + cost < dist[v])
            {
                dist[v] = dist[u] + cost;
                parent[v] = u;
                changed = 1;
            }

            /* Check v to u */
            if (dist[v] != INF &&
                dist[v] + cost < dist[u])
            {
                dist[u] = dist[v] + cost;
                parent[u] = v;
                changed = 1;
            }
        }

        if (changed == 0)
            break;
    }

    /* Print routing table */
    printf("\n========== ROUTING TABLE ==========\n\n");

    printf("%-15s %-15s %-10s\n",
           "Destination", "Distance", "Next Hop");

    printf("------------------------------------------\n");

    for (i = 0; i < g.n; i++)
    {
        int next = i;
        int steps = 0;

        if (dist[i] == INF)
        {
            printf("%-15c %-15s %-10s\n",
                   'A' + i, "INF", "-");
        }
        else if (i == g.source)
        {
            printf("%-15c %-15d %-10s\n",
                   'A' + i, dist[i], "-");
        }
        else
        {
            while (parent[next] != g.source &&
                   parent[next] != -1 &&
                   steps < g.n)
            {
                next = parent[next];
                steps++;
            }

            if (parent[next] == g.source)
            {
                printf("%-15c %-15d %-10c\n",
                       'A' + i, dist[i], 'A' + next);
            }
            else
            {
                printf("%-15c %-15d %-10s\n",
                       'A' + i, dist[i], "-");
            }
        }
    }

    printf("------------------------------------------\n");

    /* Print shortest path */
    printf("\n========== SHORTEST PATH ==========\n\n");

    printf("Source      : %c\n", 'A' + g.source);
    printf("Destination : %c\n", 'A' + g.destination);

    if (dist[g.destination] == INF)
    {
        printf("No path exists.\n");
    }
    else
    {
        int path[MAX];
        int count = 0;
        int current = g.destination;

        while (current != -1 && count < g.n)
        {
            path[count] = current;
            count++;

            if (current == g.source)
                break;

            current = parent[current];
        }

        if (count == 0 || path[count - 1] != g.source)
        {
            printf("Unable to find path.\n");
        }
        else
        {
            printf("Path        : ");

            for (i = count - 1; i >= 0; i--)
            {
                printf("%c", 'A' + path[i]);

                if (i > 0)
                    printf(" -> ");
            }

            printf("\nTotal Cost  : %d\n",
                   dist[g.destination]);
        }
    }

    close(server_fd);

    return 0;
}
