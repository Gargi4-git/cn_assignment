#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int my_strlen(char str[]) {
    int i = 0;
    while (str[i] != '\0') i++;
    return i;
}

struct Packet {
    char key[100];
    char finalData[200];
};

int main() {
    char finalData[200], key[100];
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server, transmitter;
    socklen_t transmitterLen = sizeof(transmitter);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    printf("Waiting for data...\n");

    struct Packet packet;
    int received = recvfrom(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&transmitter, &transmitterLen);
    if (received < 0) {
        perror("recvfrom failed");
        close(sock);
        return 1;
    }

    int i = 0;
    while (packet.key[i] != '\0') {
        key[i] = packet.key[i];
        i++;
    }
    key[i] = '\0';

    i = 0;
    while (packet.finalData[i] != '\0') {
        finalData[i] = packet.finalData[i];
        i++;
    }
    finalData[i] = '\0';

    printf("\nData received\n");

    int dataLen = my_strlen(finalData);
    int keyLen = my_strlen(key);
    int n = keyLen - 1;

    printf("\nReceived Data = %s", finalData);
    printf("\nKey           = %s", key);
    printf("\nn             = %d\n", n);

    int pos = 0;
    while (pos <= dataLen - keyLen) {
        while (pos < dataLen && finalData[pos] == '0') pos++;
        if (pos > dataLen - keyLen) break;

        for (int j = 0; j < keyLen; j++) {
            finalData[pos + j] = (finalData[pos + j] == key[j]) ? '0' : '1';
        }
        pos++;
    }

    int error = 0;
    for (int i = dataLen - n; i < dataLen; i++) {
        if (finalData[i] != '0') {
            error = 1;
            break;
        }
    }

    printf("\nAfter Division = %s\n", finalData);

    if (error == 0) {
        printf("All final %d bits are 0.\n", n);
        printf("Data Accepted - No Error Detected\n");
    } else {
        printf("Final %d bits are not all 0.\n", n);
        printf("Data Rejected - Error Detected\n");
    }

    close(sock);
    return 0;
}
