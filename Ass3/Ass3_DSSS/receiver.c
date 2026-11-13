#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    int sock;

    char chip[] = "10110111000";

    char received[256];

    char decrypted[50]="";
    int k=0;

    struct sockaddr_in sender,receiver;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    bind(sock,
         (struct sockaddr *)&receiver,
         sizeof(receiver));

    socklen_t len = sizeof(sender);

    recvfrom(sock,
             received,
             sizeof(received),
             0,
             (struct sockaddr *)&sender,
             &len);

    printf("Received Spread Data :\n%s\n\n",received);

    int chipLength = strlen(chip);

    int totalBlocks = strlen(received)/chipLength;

    for(int i = 0; i < totalBlocks; i++)
    {
        int match = 1;

        for(int j = 0; j < chipLength; j++)
        {
            if(received[i * chipLength + j] != chip[j])
            {
                match = 0;
                break;
            }
        }

        if(match)
            decrypted[k++] = '0';
        else
            decrypted[k++] = '1';
    }

    printf("Decrypted Data : %s\n",decrypted);

    close(sock);

    return 0;
}