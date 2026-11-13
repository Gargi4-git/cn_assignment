#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080

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

    printf("FLSM Server waiting for client...\n");

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
        subnets < 1)
    {
        snprintf(output, sizeof(output), "Invalid input.\n");
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

    int hostBits = 32 - defaultPrefix;
    uint32_t defaultMask =
        (uint32_t)(0xFFFFFFFFU << hostBits);

    uint32_t network = ipNum & defaultMask;

    int subnetBits = 0;
    int capacity = 1;

    while (capacity < subnets && subnetBits < hostBits)
    {
        capacity *= 2;
        subnetBits++;
    }

    if (capacity < subnets || subnetBits >= hostBits)
    {
        snprintf(output, sizeof(output),
                 "Requested number of subnets is too large.\n");
        send(clientSocket, output, strlen(output), 0);
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    int prefix = defaultPrefix + subnetBits;
    int remainingHostBits = 32 - prefix;

    uint32_t blockSize = 1U << remainingHostBits;
    uint32_t subnetMask =
        (uint32_t)(0xFFFFFFFFU << remainingHostBits);

    int usableHosts = (remainingHostBits >= 2)
                      ? (int)(blockSize - 2)
                      : 0;

    int offset = snprintf(output, sizeof(output),
        "\n========== FLSM RESULT ==========\n"
        "IP Address        : %s\n"
        "Number of Subnets : %d\n"
        "Default Prefix    : /%d\n"
        "New Prefix        : /%d\n"
        "Subnet Mask       : %u.%u.%u.%u\n"
        "Subnet Bits       : %d\n"
        "Host Bits         : %d\n"
        "Addresses/Subnet  : %u\n"
        "Usable Hosts      : %d\n\n",
        ip, subnets, defaultPrefix, prefix,
        (subnetMask >> 24) & 255,
        (subnetMask >> 16) & 255,
        (subnetMask >> 8) & 255,
        subnetMask & 255,
        subnetBits, remainingHostBits,
        blockSize, usableHosts);

    for (int i = 0; i < subnets && offset < (int)sizeof(output); i++)
    {
        uint32_t subnetNetwork = network + (uint32_t)i * blockSize;
        uint32_t broadcast = subnetNetwork + blockSize - 1;

        struct in_addr netAddr, broadcastAddr, firstAddr, lastAddr;

        netAddr.s_addr = htonl(subnetNetwork);
        broadcastAddr.s_addr = htonl(broadcast);
        firstAddr.s_addr = htonl(subnetNetwork + 1);
        lastAddr.s_addr = htonl(broadcast - 1);

        offset += snprintf(output + offset,
                           sizeof(output) - offset,
            "Subnet %d:\n"
            "  Network Address   : %s\n"
            "  First Usable IP   : %s\n"
            "  Last Usable IP    : %s\n"
            "  Broadcast Address : %s\n\n",
            i + 1,
            inet_ntoa(netAddr),
            inet_ntoa(firstAddr),
            inet_ntoa(lastAddr),
            inet_ntoa(broadcastAddr));
    }

    send(clientSocket, output, strlen(output), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}