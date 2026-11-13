#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    int n, f;

    printf("Enter number of packets: ");
    scanf("%d", &n);

    printf("Enter number of frequencies: ");
    scanf("%d", &f);

    int freq[f];
    int hop_seq[n];
    char message[256];

    printf("Enter frequencies:\n");

    for(int i = 0; i < f; i++)
        scanf("%d", &freq[i]);

    printf("Enter hopping sequence:\n");

    for(int i = 0; i < n; i++)
        scanf("%d", &hop_seq[i]);

    int rec_socket;

    struct sockaddr_in transmitterAddress;
    struct sockaddr_in receiverAddress;

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    rec_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(rec_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    bind(rec_socket,
         (struct sockaddr *)&receiverAddress,
         sizeof(receiverAddress));

    socklen_t len = sizeof(transmitterAddress);

    for(int i = 0; i < n; i++)
    {
        int received_frequency;

        recvfrom(rec_socket,
                 &received_frequency,
                 sizeof(received_frequency),
                 0,
                 (struct sockaddr *)&transmitterAddress,
                 &len);

        recvfrom(rec_socket,
                 message,
                 sizeof(message),
                 0,
                 (struct sockaddr *)&transmitterAddress,
                 &len);

        int expected_frequency = freq[hop_seq[i]];

        if(received_frequency == expected_frequency)
        {
            printf("Packet %d received on Frequency %d: %s\n",
                   i + 1,
                   received_frequency,
                   message);
        }
        else
        {
            printf("Packet %d received on wrong Frequency %d\n",
                   i + 1,
                   received_frequency);
        }
    }

    close(rec_socket);

    return 0;
}