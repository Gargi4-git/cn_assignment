#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/time.h>

typedef struct Node
{
    int data;
    int cycle;
    int relseq;
    struct Node *next;
} node;

typedef struct Queue
{
    node *front;
    node *rear;
} queue;

int isEmpty(queue *q) { return q->rear == NULL; }

queue *createQueue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}

node *createNode(int data,int cycle,int relseq)
{
    node *newnode = (node *)malloc(sizeof(node));
    newnode->data = data;
    newnode->cycle = cycle;
    newnode->relseq = relseq;
    newnode->next = NULL;
    return newnode;
}

void Enqueue(queue *q,int data,int cycle,int relseq)
{
    node *newnode = createNode(data,cycle,relseq);
    if(isEmpty(q)) { q->front = newnode; q->rear = newnode; }
    else { q->rear->next = newnode; q->rear = newnode; }
}

int Dequeue(queue *q)
{
    if(isEmpty(q)) return -1;
    node *temp = q->front;
    q->front = q->front->next;
    if(q->front == NULL) q->rear = NULL;
    int data = temp->data;
    free(temp);
    return data;
}

void printQueue(queue *q)
{
    node *temp = q->front;
    printf("Window: ");
    while(temp != NULL)
    {
        printf("%d(C%d:%d) ",temp->data,temp->cycle,temp->relseq);
        temp = temp->next;
    }
    printf("\n");
}

int main()
{
    int sock;
    struct sockaddr_in receiver;
    socklen_t len = sizeof(receiver);

    int tp,tt,loss,total_time=0,total_transmissions=0;
    int total_packets, window_size;

    printf("Enter number of packets to transmit: ");
    scanf("%d",&total_packets);
    printf("Enter window size (N): ");
    scanf("%d",&window_size);
    printf("Enter Transmission Time: ");
    scanf("%d",&tt);
    printf("Enter Propagation Time: ");
    scanf("%d",&tp);
    printf("Enter loss interval (every Nth packet lost once): ");
    scanf("%d",&loss);
    if(loss < 2) loss = 2;

    printf("\nTotal Packets: %d\n",total_packets);
    printf("Window Size: %d\n\n",window_size);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0) { perror("Socket creation failed"); return 1; }

    sendto(sock,&total_packets,sizeof(total_packets),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&window_size,sizeof(window_size),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&tt,sizeof(tt),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&tp,sizeof(tp),0,(struct sockaddr *)&receiver,sizeof(receiver));
    sendto(sock,&loss,sizeof(loss),0,(struct sockaddr *)&receiver,sizeof(receiver));

    struct timeval timeout;
    timeout.tv_sec = tt + 2*tp;
    timeout.tv_usec = 0;
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    queue *window = createQueue();

    int base = 1;
    int next = 1;
    int cycle = 1;

    while(base <= total_packets)
    {
        int relseq = 0;

        while(next <= total_packets && next < base + window_size)
        {
            sleep(tt);
            total_time += tt;

            sendto(sock,&next,sizeof(next),0,(struct sockaddr *)&receiver,sizeof(receiver));
            Enqueue(window,next,cycle,relseq++);
            total_transmissions++;

            printf("Timer: %d\tCycle: %d\tBase: %d\tNext: %d\tSent packet %d\n",
                   total_time,cycle,base,next,next);
            printQueue(window);

            next++;
        }

        int ack;
        int result = recvfrom(sock,&ack,sizeof(ack),0,(struct sockaddr *)&receiver,&len);

        if(result > 0)
        {
            total_time += tp;

            printf("Timer: %d\tCycle: %d\tACK %d received (cumulative)\n",
                   total_time,cycle,ack);

            base = ack + 1;
            while(!isEmpty(window) && window->front->data <= ack)
                Dequeue(window);

            printQueue(window);
            cycle++;
        }
        else
        {
            printf("Timer: %d\tCycle: %d\tTimeout! Go-Back-N: retransmitting window [%d..%d]\n",
                   total_time,cycle,base,next-1);

            int rseq = 0;
            node *temp = window->front;
            while(temp != NULL)
            {
                sleep(tt);
                total_time += tt;

                sendto(sock,&temp->data,sizeof(int),0,(struct sockaddr *)&receiver,sizeof(receiver));
                temp->cycle  = cycle + 1;
                temp->relseq = rseq++;
                total_transmissions++;

                printf("Timer: %d\tCycle: %d\tRetransmitted packet %d\n",
                       total_time,cycle+1,temp->data);

                temp = temp->next;
            }
            printQueue(window);
            cycle++;
        }
    }

    printf("\nAll packets transmitted successfully.\n");
    printf("Total time required: %d\n",total_time);
    printf("Total number of transmissions (including retransmissions): %d\n",total_transmissions);

    close(sock);
    return 0;
}