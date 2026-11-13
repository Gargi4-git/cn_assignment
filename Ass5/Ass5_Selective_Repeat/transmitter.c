#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<math.h>

int min(int x,int y) { return (x<y ? x : y); }

int total_packets, window_size, base;
int *sent, *acked;

void printWindow()
{
    printf("Window: ");
    for(int i=base;i<base+window_size && i<=total_packets;i++)
    {
        if(sent[i] && !acked[i]) printf("%d ",i);
    }
    printf("(base=%d)\n\n",base);
}

int main()
{
    int sock;
    struct sockaddr_in receiver;
    socklen_t len = sizeof(receiver);
    int tp,tt,loss,total_time=0,total_transmissions=0;

    printf("Enter Transmission Time: ");
    scanf("%d",&tt);
    printf("Enter Propagation Time: ");
    scanf("%d",&tp);
    printf("Enter loss interval: ");
    scanf("%d",&loss);
    if(loss < 2) loss = 2;

    int a = tp / tt;
    total_packets = 1 + 2 * a;
    int bits = (int)floor(log2(total_packets));
    window_size = min((int)pow(2,bits), total_packets);

    printf("\nTotal Packets: %d | Window Size: %d\n\n",total_packets,window_size);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock = socket(AF_INET,SOCK_DGRAM,0);

    sendto(sock,&total_packets,sizeof(total_packets),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&window_size,sizeof(window_size),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&tt,sizeof(tt),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&tp,sizeof(tp),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&loss,sizeof(loss),0,(struct sockaddr *)&receiver,sizeof(receiver));

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    sent     = calloc(total_packets+2,sizeof(int));
    acked    = calloc(total_packets+2,sizeof(int));
    int *deadline = calloc(total_packets+2,sizeof(int));

    base = 1;
    int next_seq = 1;

    while(base <= total_packets)
    {
        while(next_seq <= total_packets && next_seq < base + window_size)
        {
            sleep(tt);
            total_time += tt;

            sendto(sock,&next_seq,sizeof(next_seq),0,(struct sockaddr *)&receiver,sizeof(receiver));
            sent[next_seq] = 1;
            deadline[next_seq] = total_time + 2*tp;
            total_transmissions++;

            printf("t=%d  Sent %d\n",total_time,next_seq);
            next_seq++;
        }
        printWindow();

        int ack;
        int result = recvfrom(sock,&ack,sizeof(ack),0,(struct sockaddr *)&receiver,&len);

        if(result > 0)
        {
            total_time += tp;
            if(ack >= base && ack < base + window_size && sent[ack])
                acked[ack] = 1;

            printf("t=%d  ACK %d\n",total_time,ack);

            while(base <= total_packets && acked[base]) base++;
        }
        else
        {
            total_time += 1;
            for(int s = base; s < next_seq; s++)
            {
                if(sent[s] && !acked[s] && deadline[s] <= total_time)
                {
                    sleep(tt);
                    total_time += tt;
                    sendto(sock,&s,sizeof(int),0,(struct sockaddr *)&receiver,sizeof(receiver));
                    deadline[s] = total_time + 2*tp;
                    total_transmissions++;
                    printf("t=%d  Timeout -> resent %d\n",total_time,s);
                }
            }
        }
    }

    printf("\nAll packets delivered.\n");
    printf("Total time: %d | Total transmissions: %d\n",total_time,total_transmissions);

    free(sent); free(acked); free(deadline);
    close(sock);
    return 0;
}