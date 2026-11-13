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

    struct sockaddr_in receiver;
    socklen_t len = sizeof(receiver);

    printf("Enter number of packets: ");
    scanf("%d", &c.n);

    printf("Enter number of frequencies: ");
    scanf("%d", &c.f);

    if (c.n <= 0 || c.n > MAX || c.f <= 0 || c.f > MAX)
    {
        printf("Invalid input.\n");
        return 1;
    }

    printf("Enter frequencies:\n");
    for (int i = 0; i < c.f; i++)
        scanf("%d", &c.freq[i]);

    printf("Enter hopping sequence (0 to %d):\n", c.f - 1);
    for (int i = 0; i < c.n; i++)
    {
        scanf("%d", &c.hop_seq[i]);

        if (c.hop_seq[i] < 0 || c.hop_seq[i] >= c.f)
        {
            printf("Invalid hopping sequence.\n");
            return 1;
        }
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send configuration to receiver
    if (sendto(sock, &c, sizeof(c), 0,
               (struct sockaddr *)&receiver, len) != sizeof(c))
    {
        perror("Configuration sending failed");
        close(sock);
        return 1;
    }

    printf("\nEnter messages:\n");

    for (int i = 0; i < c.n; i++)
    {
        scanf("%255s", p.message);

        p.frequency = c.freq[c.hop_seq[i]];

        printf("Transmitting Packet %d on Frequency %d: %s\n",
               i + 1, p.frequency, p.message);

        if (sendto(sock, &p, sizeof(p), 0,
                   (struct sockaddr *)&receiver, len) != sizeof(p))
        {
            perror("Packet sending failed");
            close(sock);
            return 1;
        }

        sleep(2);
    }

    close(sock);

    return 0;
}
