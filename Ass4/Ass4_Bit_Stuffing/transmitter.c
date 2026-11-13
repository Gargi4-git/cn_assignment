#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FLAG "01111110"

void append(char *destination, char *source, int *pos)
{
    int i = 0;

    while (source[i] != '\0')
    {
        destination[*pos] = source[i];
        (*pos)++;
        i++;
    }

    destination[*pos] = '\0';
}

int main()
{
    int sock;
    struct sockaddr_in receiver;

    char data[100][100];
    char res[10000];

    int size;
    int pos = 0;
    int ones;

    printf("Enter number of packets: ");
    scanf("%d", &size);

    printf("Enter packets:\n");

    for (int i = 0; i < size; i++)
        scanf("%99s", data[i]);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    append(res, FLAG, &pos);

    for (int i = 0; i < size; i++)
    {
        ones = 0;

        for (int j = 0; data[i][j] != '\0'; j++)
        {
            res[pos] = data[i][j];
            pos++;

            if (data[i][j] == '1')
            {
                ones++;

                if (ones == 5)
                {
                    res[pos] = '0';
                    pos++;
                    ones = 0;
                }
            }
            else
            {
                ones = 0;
            }
        }

        append(res, FLAG, &pos);
    }

    res[pos] = '\0';

    printf("\nStuffed Data:\n");
    printf("%s\n", res);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    sendto(sock,
           res,
           pos,
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    printf("\nData sent to receiver.\n");

    close(sock);

    return 0;
}