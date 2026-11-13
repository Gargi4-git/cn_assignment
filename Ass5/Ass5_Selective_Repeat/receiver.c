#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    int sock;
    struct sockaddr_in receiver,transmitter;
    int packet, total_packets, window_size, tt, tp, loss;
    int recv_base = 1;
    int global_counter = 0;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    bind(sock,(struct sockaddr *)&receiver,sizeof(receiver));

    socklen_t len = sizeof(transmitter);

    recvfrom(sock,&total_packets,sizeof(total_packets),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&window_size,sizeof(window_size),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&tt,sizeof(tt),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&tp,sizeof(tp),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&loss,sizeof(loss),0,(struct sockaddr *)&transmitter,&len);

    printf("Receiver ready. Expecting %d packets, window %d\n\n",total_packets,window_size);

    int *buffered = calloc(total_packets+2,sizeof(int));

    while(recv_base <= total_packets)
    {
        recvfrom(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,&len);

        if(packet < recv_base)
        {
            sendto(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,len);
            printf("Packet %d re-ACKed (old)\n",packet);
            continue;
        }
        if(packet >= recv_base + window_size)
        {
            printf("Packet %d dropped (out of window)\n",packet);
            continue;
        }

        global_counter++;
        if(global_counter % loss == 0)
        {
            printf("Packet %d LOST\n",packet);
            continue;
        }

        if(!buffered[packet])
        {
            buffered[packet] = 1;
            printf("Packet %d received\n",packet);
        }

        sendto(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,len);
        printf("ACK %d sent\n",packet);

        while(recv_base <= total_packets && buffered[recv_base])
        {
            printf("  -> delivered %d\n",recv_base);
            recv_base++;
        }
    }

    printf("\nAll packets received.\n");
    free(buffered);
    close(sock);
    return 0;
}