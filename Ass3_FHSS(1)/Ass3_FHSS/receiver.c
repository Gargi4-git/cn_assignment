#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 100

struct Config
{
    int n, f;
    int freq[MAX];
    int hop_seq[MAX];
};

struct Packet
{
    int frequency;
    char message[256];
};

int main()
{
    struct Config c;
    struct Packet p;

    int sock;
    struct sockaddr_in receiver, sender;
    socklen_t len = sizeof(sender);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&receiver,
             sizeof(receiver)) < 0)
    {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    printf("Receiver waiting for configuration...\n");

    // Receive configuration
    int bytes = recvfrom(sock, &c, sizeof(c), 0,
                         (struct sockaddr *)&sender, &len);

    if (bytes != sizeof(c))
    {
        printf("Configuration receiving failed.\n");
        close(sock);
        return 1;
    }

    if (c.n <= 0 || c.n > MAX || c.f <= 0 || c.f > MAX)
    {
        printf("Invalid configuration received.\n");
        close(sock);
        return 1;
    }

    for (int i = 0; i < c.n; i++)
    {
        if (c.hop_seq[i] < 0 || c.hop_seq[i] >= c.f)
        {
            printf("Invalid hopping sequence received.\n");
            close(sock);
            return 1;
        }
    }

    printf("Configuration received successfully.\n");
    printf("Number of packets: %d\n", c.n);
    printf("Number of frequencies: %d\n", c.f);

    // Receive packets
    for (int i = 0; i < c.n; i++)
    {
        bytes = recvfrom(sock, &p, sizeof(p), 0,
                         (struct sockaddr *)&sender, &len);

        if (bytes != sizeof(p))
        {
            printf("Packet receiving failed.\n");
            close(sock);
            return 1;
        }

        p.message[255] = '\0';

        int expected_frequency = c.freq[c.hop_seq[i]];

        if (p.frequency == expected_frequency)
        {
            printf("\nPacket %d received on Frequency %d: %s\n",
                   i + 1, p.frequency, p.message);
        }
        else
        {
            printf("\nPacket %d received on wrong Frequency %d\n",
                   i + 1, p.frequency);
        }
    }

    close(sock);

    return 0;
}
