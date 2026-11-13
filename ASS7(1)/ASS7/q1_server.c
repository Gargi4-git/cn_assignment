#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int parseIP(char ip[], int parts[])
{
    int i = 0;
    int count = 0;
    int value = 0;
    int digits = 0;

    while (ip[i] != '\0')
    {
        if (ip[i] >= '0' && ip[i] <= '9')
        {
            value = value * 10 + (ip[i] - '0');
            digits++;

            if (value > 255)
                return 0;
        }
        else if (ip[i] == '.')
        {
            if (digits == 0 || count >= 3)
                return 0;

            parts[count] = value;
            count++;

            value = 0;
            digits = 0;
        }
        else
        {
            return 0;
        }

        i++;
    }

    if (digits == 0 || count != 3)
        return 0;

    parts[count] = value;

    return 1;
}

void printIP(int ip[])
{
    printf("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

void calculateIP(char ip[])
{
    int parts[4];
    int mask[4];
    int net[4];
    int host[4];
    int last[4];
    int lba[4];

    int prefix = 0;
    int networkBits;
    int hostBits;
    int networks = 1;
    int hosts = 1;
    char className;

    if (!parseIP(ip, parts))
    {
        printf("Invalid IP Address!\n");
        return;
    }

    int a = parts[0];

    /* Determine IP class */
    if (a >= 1 && a <= 126)
    {
        className = 'A';
        prefix = 8;
        networks = 128;
    }
    else if (a >= 128 && a <= 191)
    {
        className = 'B';
        prefix = 16;
        networks = 16384;
    }
    else if (a >= 192 && a <= 223)
    {
        className = 'C';
        prefix = 24;
        networks = 2097152;
    }
    else
    {
        printf("Class D, Class E, or reserved IP address.\n");
        return;
    }

    networkBits = prefix;
    hostBits = 32 - prefix;

    /* Calculate subnet mask without using << */
    for (int i = 0; i < 4; i++)
    {
        if (prefix >= 8)
        {
            mask[i] = 255;
            prefix = prefix - 8;
        }
        else if (prefix > 0)
        {
            int value = 1;

            for (int j = 0; j < 8 - prefix; j++)
            {
                value = value * 2;
            }

            mask[i] = 256 - value;
            prefix = 0;
        }
        else
        {
            mask[i] = 0;
        }
    }

    /* Calculate Network ID, Host ID and DBA */
    for (int i = 0; i < 4; i++)
    {
        net[i] = parts[i] & mask[i];

        host[i] = parts[i] & (255 - mask[i]);

        last[i] = net[i] | (255 - mask[i]);
    }

    /* Calculate LBA: Limited Broadcast Address */
    for (int i = 0; i < 4; i++)
    {
        lba[i] = 255;
    }

    /* Calculate number of usable hosts */
    for (int i = 0; i < hostBits; i++)
    {
        hosts = hosts * 2;
    }

    hosts = hosts - 2;

    /* Print results */
    printf("\n========== IP ADDRESS CALCULATOR ==========\n\n");

    printf("+----------------------+----------------------+\n");
    printf("| Parameter            | Value                |\n");
    printf("+----------------------+----------------------+\n");

    printf("| IP Address           | ");
    printIP(parts);
    printf("\n");

    printf("| Class                | %c\n", className);

    printf("| Network ID           | ");
    printIP(net);
    printf("\n");

    printf("| Host ID              | ");
    printIP(host);
    printf("\n");

    printf("| Number of Networks   | %d\n", networks);
    printf("| Number of Hosts      | %d\n", hosts);
    printf("| Network Bits         | %d\n", networkBits);
    printf("| Host Bits            | %d\n", hostBits);

    printf("| LBA                  | ");
    printIP(lba);
    printf("\n");

    printf("| DBA                  | ");
    printIP(last);
    printf("\n");

    printf("+----------------------+----------------------+\n");
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server;
    char ip[100];
    int n;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        printf("Listen failed\n");
        close(server_fd);
        return 1;
    }

    printf("Server listening on port 8080...\n");

    while (1)
    {
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            printf("Accept failed\n");
            continue;
        }

        n = recv(client_fd, ip, sizeof(ip) - 1, 0);

        if (n > 0)
        {
            ip[n] = '\0';

            /* Remove newline or carriage return */
            for (int i = 0; ip[i] != '\0'; i++)
            {
                if (ip[i] == '\n' || ip[i] == '\r')
                {
                    ip[i] = '\0';
                    break;
                }
            }

            calculateIP(ip);
        }
        else
        {
            printf("Failed to receive IP address\n");
        }

        close(client_fd);
    }

    close(server_fd);

    return 0;
}
