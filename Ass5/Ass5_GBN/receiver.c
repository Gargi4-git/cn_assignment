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

    int packet;
    int total_packets, window_size;
    int tt,tp,loss;
    int expected = 1;
    int counter = 0;

    int lost[100000];
    for(int i=0;i<100000;i++) lost[i] = 0;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0) { perror("Socket creation failed"); return 1; }

    bind(sock,(struct sockaddr *)&receiver,sizeof(receiver));

    socklen_t len = sizeof(transmitter);

    recvfrom(sock,&total_packets,sizeof(total_packets),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&window_size,sizeof(window_size),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&tt,sizeof(tt),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&tp,sizeof(tp),0,(struct sockaddr *)&transmitter,&len);
    recvfrom(sock,&loss,sizeof(loss),0,(struct sockaddr *)&transmitter,&len);

    printf("Receiver Started (Go-Back-N)\n");
    printf("Expecting %d packets, window size %d, loss interval = %d (relative to last loss recovery)\n\n",
           total_packets,window_size,loss);

    while(expected <= total_packets)
    {
        recvfrom(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,&len);

        if(packet == expected)
        {
            if(lost[packet] == 0)
            {
                counter++;

                if(counter == loss)
                {
                    lost[packet] = 1;
                    printf("Packet %d LOST (simulated, position %d in streak)\n\n",
                           packet,counter);
                    continue;
                }

                printf("Packet %d received\n",packet);
                sendto(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,len);
                printf("ACK %d sent (cumulative)\n\n",packet);
                expected++;
            }
            else
            {
                printf("Packet %d received (recovered after earlier loss)\n",packet);
                sendto(sock,&packet,sizeof(packet),0,(struct sockaddr *)&transmitter,len);
                printf("ACK %d sent (cumulative)\n\n",packet);
                expected++;
                counter = 1;
            }
        }
        else
        {
            printf("Packet %d discarded (out of order, expected %d)\n\n",
                   packet,expected);
        }
    }

    printf("All packets received successfully.\n");
    close(sock);
    return 0;
}