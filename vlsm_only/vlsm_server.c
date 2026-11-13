#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080
#define MAX_SUBNETS 50

struct Subnet
{
    int hostsRequired;
    int hostBits;
    int prefix;
    uint32_t blockSize;
    uint32_t network;
    uint32_t broadcast;
};

int compareHosts(const void *a, const void *b)
{
    const struct Subnet *x = a;
    const struct Subnet *y = b;

    return y->hostsRequired - x->hostsRequired;
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLen = sizeof(clientAddress);

    char input[100];
    char output[4096];
    char ip[50];

    int subnets;
    int received;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("VLSM Server waiting for client...\n");

    clientSocket = accept(serverSocket,
                          (struct sockaddr *)&clientAddress,
                          &clientLen);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    received = recv(clientSocket, input, sizeof(input) - 1, 0);

    if (received <= 0)
    {
        printf("Failed to receive client data.\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    input[received] = '\0';

    if (sscanf(input, "%49s %d", ip, &subnets) != 2 ||
        subnets < 1 || subnets > MAX_SUBNETS)
    {
        snprintf(output, sizeof(output),
                 "Invalid input or subnet count.\n");
        send(clientSocket, output, strlen(output), 0);
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    struct in_addr address;

    if (inet_pton(AF_INET, ip, &address) != 1)
    {
        snprintf(output, sizeof(output), "Invalid IPv4 address.\n");
        send(clientSocket, output, strlen(output), 0);
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    uint32_t ipNum = ntohl(address.s_addr);

    int firstOctet = (ipNum >> 24) & 255;
    int defaultPrefix;

    if (firstOctet >= 1 && firstOctet <= 126)
        defaultPrefix = 8;
    else if (firstOctet >= 128 && firstOctet <= 191)
        defaultPrefix = 16;
    else if (firstOctet >= 192 && firstOctet <= 223)
        defaultPrefix = 24;
    else
    {
        snprintf(output, sizeof(output),
                 "This example supports class A, B, and C IPv4 addresses.\n");
        send(clientSocket, output, strlen(output), 0);
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    int parentHostBits = 32 - defaultPrefix;
    uint32_t parentSize = 1U << parentHostBits;
    uint32_t parentMask =
        (uint32_t)(0xFFFFFFFFU << parentHostBits);

    uint32_t parentNetwork = ipNum & parentMask;
    uint32_t parentBroadcast = parentNetwork + parentSize - 1;

    /*
       Example host requirements.
       Modify these values according to your practical question.
       There must be at least 'subnets' entries.
    */
    int hostRequirements[MAX_SUBNETS] =
    {
        100, 50, 25, 10, 5, 2
    };

    if (subnets > 6)
    {
        snprintf(output, sizeof(output),
                 "This example has host requirements for at most 6 subnets.\n");
        send(clientSocket, output, strlen(output), 0);
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    struct Subnet subnet[MAX_SUBNETS];

    for (int i = 0; i < subnets; i++)
    {
        subnet[i].hostsRequired = hostRequirements[i];

        int hostBits = 0;
        uint32_t blockSize = 1;

        while (blockSize < (uint32_t)(hostRequirements[i] + 2))
        {
            blockSize *= 2;
            hostBits++;
        }

        subnet[i].hostBits = hostBits;
        subnet[i].prefix = 32 - hostBits;
        subnet[i].blockSize = blockSize;
    }

    qsort(subnet, subnets, sizeof(struct Subnet), compareHosts);

    uint32_t currentNetwork = parentNetwork;

    for (int i = 0; i < subnets; i++)
    {
        uint32_t blockSize = subnet[i].blockSize;

        currentNetwork =
            (currentNetwork + blockSize - 1) / blockSize * blockSize;

        subnet[i].network = currentNetwork;
        subnet[i].broadcast = currentNetwork + blockSize - 1;

        if (subnet[i].network < parentNetwork ||
            subnet[i].broadcast > parentBroadcast)
        {
            snprintf(output, sizeof(output),
                     "VLSM allocation does not fit inside the parent network.\n");
            send(clientSocket, output, strlen(output), 0);
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        currentNetwork = subnet[i].broadcast + 1;
    }

    int offset = snprintf(output, sizeof(output),
        "\n========== VLSM RESULT ==========\n"
        "IP Address        : %s\n"
        "Parent Prefix     : /%d\n"
        "Number of Subnets : %d\n\n",
        ip, defaultPrefix, subnets);

    for (int i = 0; i < subnets && offset < (int)sizeof(output); i++)
    {
        uint32_t mask =
            (uint32_t)(0xFFFFFFFFU << subnet[i].hostBits);

        struct in_addr netAddr, firstAddr, lastAddr, broadcastAddr;

        netAddr.s_addr = htonl(subnet[i].network);
        firstAddr.s_addr = htonl(subnet[i].network + 1);
        lastAddr.s_addr = htonl(subnet[i].broadcast - 1);
        broadcastAddr.s_addr = htonl(subnet[i].broadcast);

        offset += snprintf(output + offset, sizeof(output) - offset,
            "Subnet %d:\n"
            "  Hosts Required    : %d\n"
            "  Prefix            : /%d\n"
            "  Subnet Mask       : %u.%u.%u.%u\n"
            "  Network Address   : %s\n"
            "  First Usable IP   : %s\n"
            "  Last Usable IP    : %s\n"
            "  Broadcast Address : %s\n"
            "  Total Addresses   : %u\n\n",
            i + 1,
            subnet[i].hostsRequired,
            subnet[i].prefix,
            (mask >> 24) & 255,
            (mask >> 16) & 255,
            (mask >> 8) & 255,
            mask & 255,
            inet_ntoa(netAddr),
            inet_ntoa(firstAddr),
            inet_ntoa(lastAddr),
            inet_ntoa(broadcastAddr),
            subnet[i].blockSize);
    }

    send(clientSocket, output, strlen(output), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}