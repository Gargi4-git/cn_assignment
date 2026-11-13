#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int length(char *str)
{
    int i = 0;

    while(str[i] != '\0')
        i++;

    return i;
}

int main()
{
    int n, f;

    printf("Enter number of packets: ");
    scanf("%d", &n);

    printf("Enter number of frequencies: ");
    scanf("%d", &f);

    int freq[f];
    int hop_seq[n];
    char msg[n][256];

    printf("Enter frequencies:\n");

    for(int i = 0; i < f; i++)
        scanf("%d", &freq[i]);

    printf("Enter hopping sequence:\n");

    for(int i = 0; i < n; i++)
        scanf("%d", &hop_seq[i]);

    printf("Enter messages:\n");

    for(int i = 0; i < n; i++)
        scanf("%255s", msg[i]);

    int tran_socket;

    struct sockaddr_in receiverAddress;

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    tran_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(tran_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    for(int i = 0; i < n; i++)
    {
        int current_frequency = freq[hop_seq[i]];

        printf("Transmitting Packet %d on Frequency %d: %s\n",
               i + 1,
               current_frequency,
               msg[i]);

        sendto(tran_socket,
               &current_frequency,
               sizeof(current_frequency),
               0,
               (struct sockaddr *)&receiverAddress,
               sizeof(receiverAddress));

        sendto(tran_socket,
               msg[i],
               length(msg[i]) + 1,
               0,
               (struct sockaddr *)&receiverAddress,
               sizeof(receiverAddress));

        sleep(2);
    }

    close(tran_socket);

    return 0;
}