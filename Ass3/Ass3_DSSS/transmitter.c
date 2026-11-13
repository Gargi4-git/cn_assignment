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

    struct sockaddr_in receiver;

    char chip[] = "10110111000";
    char data[100];

    printf("Enter binary data (0s and 1s): ");
    scanf("%s", data);

    char spread[256] = "";
    int k=0;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Actual Data : %s\n",data);

    for(int i=0;i<strlen(data);i++)
    {
        for(int j=0;j<strlen(chip);j++)
        {
            if(data[i] == '0')
                spread[k++] = chip[j];
            else{
                if(chip[j]=='1')
                    spread[k++]='0';
                else
                    spread[k++]='1';
            }
        }
    }

    printf("\nSpread Data :\n%s\n",spread);

    sock = socket(AF_INET,SOCK_DGRAM,0);

    sendto(sock,
           spread,
           strlen(spread)+1,
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    close(sock);

    return 0;
}