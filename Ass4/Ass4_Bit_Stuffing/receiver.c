#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FLAG "01111110"

int length(char *str)
{
    int i = 0;

    while (str[i] != '\0')
        i++;

    return i;
}

int check_at(char *data, int pos, char *pattern)
{
    int i = 0;

    while (pattern[i] != '\0')
    {
        if (data[pos + i] == '\0' || data[pos + i] != pattern[i])
            return 0;

        i++;
    }

    return 1;
}

int main()
{
    int sock;

    struct sockaddr_in receiver;
    struct sockaddr_in sender;

    char received[10000];
    char decrypted[10000];

    int sender_len = sizeof(sender);
    int n;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock,
             (struct sockaddr *)&receiver,
             sizeof(receiver)) < 0)
    {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    printf("Receiver waiting for data...\n");

    n = recvfrom(sock,
                 received,
                 9999,
                 0,
                 (struct sockaddr *)&sender,
                 (socklen_t *)&sender_len);

    if (n < 0)
    {
        perror("Receive failed");
        close(sock);
        return 1;
    }

    received[n] = '\0';

    printf("\nReceived Stuffed Data:\n");
    printf("%s\n", received);

    int i = length(FLAG);
    int packet = 1;

    printf("\nDecrypted Data:\n");

    while (i < n)
    {
        int pos = 0;
        int ones = 0;

        while (i < n)
        {
            if (check_at(received, i, FLAG))
            {
                i = i + length(FLAG);
                break;
            }

            decrypted[pos] = received[i];
            pos++;

            if (received[i] == '1')
            {
                ones++;

                if (ones == 5)
                {
                    i++;

                    if (i < n && received[i] == '0')
                    {
                        ones = 0;
                    }
                }
            }
            else
            {
                ones = 0;
            }

            i++;
        }

        decrypted[pos] = '\0';

        if (pos > 0)
        {
            printf("Packet %d: %s\n", packet, decrypted);
            packet++;
        }
    }

    close(sock);

    return 0;
}