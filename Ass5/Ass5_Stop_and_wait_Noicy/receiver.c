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
    struct sockaddr_in sender, receiver;
    socklen_t len = sizeof(sender);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family      = AF_INET;
    receiver.sin_port        = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0) {
        perror("bind");
        exit(1);
    }

    recvfrom(sock, &size, sizeof(size), 0, (struct sockaddr *)&sender, &len);
    recvfrom(sock, &tt,   sizeof(tt),   0, (struct sockaddr *)&sender, &len);
    recvfrom(sock, &tp,   sizeof(tp),   0, (struct sockaddr *)&sender, &len);
    recvfrom(sock, &loss, sizeof(loss), 0, (struct sockaddr *)&sender, &len);

    printf("Expecting %d packets (tt=%d, tp=%d, every %dth packet lost once)\n",
           size, tt, tp, loss);

    int expected    = 1;
    int lastDropped = 0;

    while (expected <= size) {
        int frame = 0;

        // Blocks here regardless of whether this is an original
        // transmission or a retransmission the sender issued after
        // its own timers (Timer1/Timer2/Timer3) expired -- from this
        // side, both look identical: just "a frame arrived".
        recvfrom(sock, &frame, sizeof(frame), 0, (struct sockaddr *)&sender, &len);
        sleep(tp);

        if (frame % loss == 0 && frame != lastDropped) {
            lastDropped = frame;
            printf("Packet %d corrupted/lost on the channel - no ack sent\n", frame);
            continue;
        }

        if (frame == expected) {
            printf("Received packet %d from transmitter\n", frame);
            expected++;
        } else {
            printf("Duplicate packet %d received, re-acknowledging\n", frame);
        }

        sleep(tt);
        sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr *)&sender, len);
    }

    printf("All %d packets received\n", size);
    close(sock);
    return 0;
}