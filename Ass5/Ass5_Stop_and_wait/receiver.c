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
    struct sockaddr_in sender,receiver;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    bind(sock,
         (struct sockaddr *)&receiver,
         sizeof(receiver));

    socklen_t len = sizeof(sender);

    int buffer;
    int tt,tp,size;

    recvfrom(sock,&size,sizeof(size),0,(struct sockaddr *)&sender,&len);
    recvfrom(sock,&tt,sizeof(tt),0,(struct sockaddr *)&sender,&len);
    recvfrom(sock,&tp,sizeof(tp),0,(struct sockaddr *)&sender,&len);

    int i=0,ack=1;
    while(i<size){
        sleep(tp);
        recvfrom(sock,&buffer,sizeof(buffer),0,(struct sockaddr *)&sender,&len);
        printf("Received %d packet from transmitter\n",buffer);
        sleep(tt);
        sendto(sock,&ack,sizeof(ack),0,(struct sockaddr *)&sender,len);
        i++;
    }

    close(sock);

    return 0;
}