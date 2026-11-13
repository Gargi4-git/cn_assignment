
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_OUTPUT 30000

/* Calculate 2 raised to a power without using << */
int power2(int power)
{
    int result = 1;

    while (power > 0)
    {
        result = result * 2;
        power--;
    }

    return result;
}

/* Read an IP address and subnet count from the client's message */
int parseInput(char input[], int ip[])
{
    int i = 0;
    int octet = 0;
    int value = 0;
    int digits = 0;
    int count = 0;

    /* Read the four IP octets */
    while (input[i] != '\0' &&
           input[i] != ' ' &&
           input[i] != '\n')
    {
        if (input[i] >= '0' && input[i] <= '9')
        {
            value = value * 10 + (input[i] - '0');
            digits++;
        }
        else if (input[i] == '.')
        {
            if (digits == 0 || octet >= 3 || value > 255)
                return -1;

            ip[octet] = value;
            octet++;
            value = 0;
            digits = 0;
        }
        else
        {
            return -1;
        }

        i++;
    }

    if (digits == 0 || octet != 3 || value > 255)
        return -1;

    ip[3] = value;

    /* Skip spaces before the subnet count */
    while (input[i] == ' ' || input[i] == '\n')
        i++;

    /* Read the number of subnets */
    while (input[i] >= '0' && input[i] <= '9')
    {
        count = count * 10 + (input[i] - '0');
        i++;
    }

    if (count < 1 || count > 64)
        return -1;

    /* Only spaces/newlines may follow the subnet count */
    while (input[i] == ' ' || input[i] == '\n' ||
           input[i] == '\r')
    {
        i++;
    }

    if (input[i] != '\0')
        return -1;

    return count;
}

/* Identify the IP class and its default prefix */
int getDefaultPrefix(int firstOctet)
{
    if (firstOctet >= 1 && firstOctet <= 126)
        return 8;       /* Class A */

    if (firstOctet >= 128 && firstOctet <= 191)
        return 16;      /* Class B */

    if (firstOctet >= 192 && firstOctet <= 223)
        return 24;      /* Class C */

    return -1;
}

/* Calculate the subnet mask from the prefix */
void getMask(int prefix, int mask[])
{
    int i;
    int remaining;

    for (i = 0; i < 4; i++)
    {
        remaining = prefix - (i * 8);

        if (remaining >= 8)
            mask[i] = 255;
        else if (remaining <= 0)
            mask[i] = 0;
        else
            mask[i] = 256 - power2(8 - remaining);
    }
}

/* Convert the four octets into a printable IP string */
void printIP(char output[], int ip[])
{
    sprintf(output, "%d.%d.%d.%d",
            ip[0], ip[1], ip[2], ip[3]);
}

/* Calculate the classful network address */
void getBaseNetwork(int ip[], int prefix, int network[])
{
    int i;
    int fullOctets = prefix / 8;

    for (i = 0; i < 4; i++)
    {
        if (i < fullOctets)
            network[i] = ip[i];
        else
            network[i] = 0;
    }
}

/* Calculate a subnet's network ID */
void getSubnetNetwork(int base[], int prefix,
                      int subnetIndex, int network[])
{
    int i;
    int fullOctets = prefix / 8;
    int remaining = prefix % 8;
    int boundary;
    int blockSize;
    int amount;
    int total;

    for (i = 0; i < 4; i++)
        network[i] = base[i];

    if (remaining == 0)
    {
        boundary = fullOctets - 1;
        blockSize = 1;
    }
    else
    {
        boundary = fullOctets;
        blockSize = power2(8 - remaining);
    }

    amount = subnetIndex * blockSize;

    /* Add the subnet offset and carry between octets */
    while (amount > 0 && boundary >= 0)
    {
        total = network[boundary] + amount;

        network[boundary] = total % 256;
        amount = total / 256;

        boundary--;
    }
}

/* Calculate the broadcast address */
void getBroadcast(int network[], int prefix, int broadcast[])
{
    int i;
    int fullOctets = prefix / 8;
    int remaining = prefix % 8;
    int blockSize;

    for (i = 0; i < 4; i++)
        broadcast[i] = network[i];

    if (remaining > 0)
    {
        blockSize = power2(8 - remaining);

        broadcast[fullOctets] =
            network[fullOctets] + blockSize - 1;

        for (i = fullOctets + 1; i < 4; i++)
            broadcast[i] = 255;
    }
    else
    {
        for (i = fullOctets; i < 4; i++)
            broadcast[i] = 255;
    }
}

/* Increase an IP address by one */
void incrementIP(int ip[])
{
    int i;

    for (i = 3; i >= 0; i--)
    {
        if (ip[i] < 255)
        {
            ip[i]++;
            return;
        }

        ip[i] = 0;
    }
}

/* Decrease an IP address by one */
void decrementIP(int ip[])
{
    int i;

    for (i = 3; i >= 0; i--)
    {
        if (ip[i] > 0)
        {
            ip[i]--;
            return;
        }

        ip[i] = 255;
    }
}

/* Send the complete response over TCP */
void sendAll(int client, char message[])
{
    int total = 0;
    int length = strlen(message);
    int sent;

    while (total < length)
    {
        sent = send(client, message + total,
                    length - total, 0);

        if (sent <= 0)
            return;

        total += sent;
    }
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    char input[100];
    char output[MAX_OUTPUT];
    char ipText[20];
    char firstText[20];
    char lastText[20];
    char usableFirstText[20];
    char usableLastText[20];
    char maskText[20];

    int ip[4];
    int base[4];
    int network[4];
    int broadcast[4];
    int usableFirst[4];
    int usableLast[4];
    int mask[4];

    int subnetCount;
    int defaultPrefix;
    int borrowedBits = 0;
    int possibleSubnets = 1;
    int newPrefix;
    int i;
    int used = 0;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

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

    printf("Server waiting for client on port %d...\n", PORT);

    clientSocket = accept(serverSocket,
                          (struct sockaddr *)&clientAddress,
                          &clientLength);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    memset(input, 0, sizeof(input));

    int received = recv(clientSocket, input,
                        sizeof(input) - 1, 0);

    if (received <= 0)
    {
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    input[received] = '\0';

    subnetCount = parseInput(input, ip);

    if (subnetCount == -1)
    {
        sendAll(clientSocket,
                "Invalid input. Enter a valid IP and 1-64 subnets.\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    defaultPrefix = getDefaultPrefix(ip[0]);

    if (defaultPrefix == -1)
    {
        sendAll(clientSocket,
                "Invalid IP class. Use a Class A, B, or C address.\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    /* Find the number of bits to borrow */
    while (possibleSubnets < subnetCount)
    {
        possibleSubnets = possibleSubnets * 2;
        borrowedBits++;
    }

    newPrefix = defaultPrefix + borrowedBits;

    /* Keep at least two usable host addresses per subnet */
    if (newPrefix > 30)
    {
        sendAll(clientSocket,
                "Not enough host bits for this subnet count.\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    /* Calculate the original classful network */
    getBaseNetwork(ip, defaultPrefix, base);

    /* Calculate the new subnet mask */
    getMask(newPrefix, mask);

    printIP(ipText, ip);
    printIP(maskText, mask);

    used += snprintf(output + used, sizeof(output) - used,
                     "\n========== FIXED LENGTH SUBNETTING ==========\n"
                     "Given IP       : %s\n"
                     "Required subnets: %d\n"
                     "Default prefix : /%d\n"
                     "Borrowed bits  : %d\n"
                     "New prefix     : /%d\n"
                     "Subnet mask    : %s\n"
                     "============================================\n",
                     ipText, subnetCount, defaultPrefix,
                     borrowedBits, newPrefix, maskText);

    for (i = 0; i < subnetCount; i++)
    {
        getSubnetNetwork(base, newPrefix, i, network);

        getBroadcast(network, newPrefix, broadcast);

        /* Usable range excludes network and broadcast addresses */
        for (int j = 0; j < 4; j++)
            usableFirst[j] = network[j];

        for (int j = 0; j < 4; j++)
            usableLast[j] = broadcast[j];

        incrementIP(usableFirst);
        decrementIP(usableLast);

        printIP(firstText, network);
        printIP(lastText, broadcast);
        printIP(usableFirstText, usableFirst);
        printIP(usableLastText, usableLast);

        used += snprintf(output + used, sizeof(output) - used,
                         "\nSubnet %d\n"
                         "Network ID     : %s\n"
                         "First IP        : %s\n"
                         "Last IP         : %s\n"
                         "Usable IP range : %s - %s\n"
                         "Subnet mask     : %s\n",
                         i + 1, firstText, firstText, lastText,
                         usableFirstText, usableLastText, maskText);

        if (used >= MAX_OUTPUT)
            break;
    }

    sendAll(clientSocket, output);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
