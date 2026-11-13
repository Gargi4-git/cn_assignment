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
    int size=0,tp,tt,total_time=0;

    printf("Enter number of packets to transmit: ");
    scanf("%d",&size);
    printf("Enter Transmission Time: ");
    scanf("%d",&tt);
    printf("Enter Propagation Time: ");
    scanf("%d",&tp);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = socket(AF_INET,SOCK_DGRAM,0);

    int buffer=1;
    sendto(sock,&size,sizeof(size),0,(struct sockaddr *)&receiver,
           sizeof(receiver));
    sendto(sock,&tt,sizeof(tt),0,(struct sockaddr *)&receiver,
           sizeof(receiver));
    sendto(sock,&tp,sizeof(tp),0,(struct sockaddr *)&receiver,
           sizeof(receiver));

    int i=0,ack=0;
    socklen_t len = sizeof(receiver);

    do{
        sleep(tt);
        printf("Packet %d send to receiver\n",buffer);
        sendto(sock,&buffer,sizeof(buffer),0,(struct sockaddr *)&receiver,
           sizeof(receiver));
        recvfrom(sock,
             &ack,
             sizeof(ack),
             0,
             (struct sockaddr *)&receiver,
             &len);
        if(ack==1){
        printf("Packet %d received from receiver\n",buffer);
        buffer++;
        }
        total_time+=2*tt+2*tp;

    }while(buffer<=size && ack==1);
    printf("Total time required: \n%d",total_time);
    close(sock);

    return 0;
}