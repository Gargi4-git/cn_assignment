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
    char received[256],data[256];
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

    printf("Received Data :\n%s\n\n",received);

    int i=0,k=0;
    int n;
    while(received[i]!='\0'){
        n=received[i]-'0';
        for (int j=0;j<n-1;j++,k++){
            data[k]=received[i+j+1];
        }
        i+=n;
    }

    data[k]='\0';
    printf("The actual data is:%s",data);

    close(sock);

    return 0;
}