#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    int sock, size = 0, tt = 0, tp = 0, loss = 2;
    struct sockaddr_in receiver, from;
    socklen_t len = sizeof(from);

    printf("Enter number of packets to transmit: ");
    scanf("%d", &size);
    printf("Enter Transmission Time: ");
    scanf("%d", &tt);
    printf("Enter Propagation Time: ");
    scanf("%d", &tp);
    printf("Enter loss factor (every Nth packet is lost): ");
    scanf("%d", &loss);
    if (loss < 2) loss = 2;

    int rtt = 2 * tt + 2 * tp;
    int clk = 0;   // single running clock, driven by the rtt formula

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family      = AF_INET;
    receiver.sin_port        = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, &size, sizeof(size), 0, (struct sockaddr *)&receiver, sizeof(receiver));
    sendto(sock, &tt,   sizeof(tt),   0, (struct sockaddr *)&receiver, sizeof(receiver));
    sendto(sock, &tp,   sizeof(tp),   0, (struct sockaddr *)&receiver, sizeof(receiver));
    sendto(sock, &loss, sizeof(loss), 0, (struct sockaddr *)&receiver, sizeof(receiver));

    printf("\n----------------------------------------\n");

    int frame = 1;
    while (frame <= size) {
        int send_start = clk;

        sleep(tt);
        clk = send_start + tt;
        printf("[T=%3d] Packet %d sent\n", clk, frame);
        sendto(sock, &frame, sizeof(frame), 0,
               (struct sockaddr *)&receiver, sizeof(receiver));

        // Real wait: just to detect whether an ack actually shows up.
        int ack = -1, n, gotAck = 0;
        for (int i = 0; i < rtt; i++) {
            n = recvfrom(sock, &ack, sizeof(ack), MSG_DONTWAIT,
                         (struct sockaddr *)&from, &len);
            if (n >= 0) { gotAck = 1; break; }
            sleep(1);
        }

        if (gotAck && ack == frame) {
            clk = send_start + rtt;                 // ---- Timer1 success ----
            printf("[T=%3d] Ack %d received\n", clk, ack);
            frame++;
            continue;
        }

        // ---- Timer1 expired ----
        clk = send_start + rtt;
        printf("[T=%3d] No ack for packet %d, waiting longer\n", clk, frame);

        // ---- Timer2 expired (cumulative 3*RTT) ----
        clk = send_start + 3 * rtt;
        printf("[T=%3d] Timeout, retransmitting packet %d\n", clk, frame);

        int retransmit_start = clk;
        sendto(sock, &frame, sizeof(frame), 0,
               (struct sockaddr *)&receiver, sizeof(receiver));

        int ack2 = -1, gotAck2 = 0;
        for (int i = 0; i < rtt; i++) {
            n = recvfrom(sock, &ack2, sizeof(ack2), MSG_DONTWAIT,
                         (struct sockaddr *)&from, &len);
            if (n >= 0) { gotAck2 = 1; break; }
            sleep(1);
        }

        clk = retransmit_start + rtt;                // ---- Timer3 window ----
        if (gotAck2 && ack2 == frame) {
            printf("[T=%3d] Ack %d received (retransmit)\n", clk, ack2);
            frame++;
        } else {
            printf("[T=%3d] Retransmit also timed out, retrying packet %d\n", clk, frame);
            // frame unchanged -> loop restarts Timer1 from this new clk
        }
    }

    printf("----------------------------------------\n");
    printf("Total time required: %d\n", clk);
    close(sock);
    return 0;
}