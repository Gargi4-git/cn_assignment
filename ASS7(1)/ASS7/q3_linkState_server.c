#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 26
#define INF 1000000000

int graph[MAX][MAX];
int dist[MAX], visited[MAX], nextHop[MAX];

void dijkstra(int n, int src)
{
    for (int i = 0; i < n; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        nextHop[i] = -1;
    }

    dist[src] = 0;

    for (int count = 0; count < n; count++)
    {
        int u = -1;
        int minimum = INF;

        for (int i = 0; i < n; i++)
        {
            if (!visited[i] && dist[i] < minimum)
            {
                minimum = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (graph[u][v] != INF && !visited[v])
            {
                if (dist[u] + graph[u][v] < dist[v])
                {
                    dist[v] = dist[u] + graph[u][v];

                    if (u == src)
                        nextHop[v] = v;
                    else
                        nextHop[v] = nextHop[u];
                }
            }
        }
    }
}

void displayTable(int n, int src)
{
    printf("\nLINK STATE ROUTING TABLE\n");
    printf("------------------------------------------\n");
    printf("%-15s %-15s %-15s\n",
           "Destination", "Distance", "Next Hop");
    printf("------------------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        if (dist[i] == INF)
        {
            printf("%-15c %-15s %-15s\n",
                   'A' + i, "INF", "N/A");
        }
        else if (i == src)
        {
            printf("%-15c %-15d %-15s\n",
                   'A' + i, 0, "-");
        }
        else
        {
            printf("%-15c %-15d %-15c\n",
                   'A' + i, dist[i], 'A' + nextHop[i]);
        }
    }

    printf("------------------------------------------\n");
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;

    char buffer[4096];
    char *line;
    int n, edges, src;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Link State Routing Server Started...\n");

    while (1)
    {
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
            continue;

        // Receive the complete message from the client.
        int total = 0;
        ssize_t received;

        while ((received = recv(client_fd,
                    buffer + total,
                    sizeof(buffer) - 1 - total, 0)) > 0)
        {
            total += received;

            if (total >= (int)sizeof(buffer) - 1)
                break;
        }

        close(client_fd);

        if (total == 0)
            continue;

        buffer[total] = '\0';

        // Read number of nodes and edges.
        line = strtok(buffer, "\n");

        if (!line || sscanf(line, "%d %d", &n, &edges) != 2 ||
            n < 1 || n > MAX || edges < 0 ||
            edges > n * (n - 1) / 2)
        {
            printf("Invalid graph input.\n");
            continue;
        }

        // Initialize graph.
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                graph[i][j] = (i == j) ? 0 : INF;
            }
        }

        int valid = 1;

        // Read each edge: source destination weight.
        for (int i = 0; i < edges; i++)
        {
            line = strtok(NULL, "\n");

            char u, v;
            int w;

            if (!line || sscanf(line, " %c %c %d", &u, &v, &w) != 3 ||
                u < 'A' || u >= 'A' + n ||
                v < 'A' || v >= 'A' + n ||
                u == v || w < 0 || w >= INF)
            {
                valid = 0;
                break;
            }

            int x = u - 'A';
            int y = v - 'A';

            // Keep the smallest cost for duplicate edges.
            if (w < graph[x][y])
            {
                graph[x][y] = w;
                graph[y][x] = w;
            }
        }

        if (!valid)
        {
            printf("Invalid edge input.\n");
            continue;
        }

        // Read source node.
        line = strtok(NULL, "\n");

        char source;

        if (!line || sscanf(line, " %c", &source) != 1 ||
            source < 'A' || source >= 'A' + n)
        {
            printf("Invalid source node.\n");
            continue;
        }

        src = source - 'A';

        printf("\nReceived graph with %d nodes and %d edges.\n",
               n, edges);

        dijkstra(n, src);
        displayTable(n, src);
    }

    close(server_fd);
    return 0;
}
