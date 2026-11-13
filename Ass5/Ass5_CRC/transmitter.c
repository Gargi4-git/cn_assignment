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

void my_strcpy(char destination[], char source[]) {
    int i = 0;
    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
}

struct Packet {
    char key[100];
    char finalData[200];
};

int main() {
    char data[100], key[100], newData[200], remainder[100];
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in receiver;
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter Data: ");
    scanf("%s", data);
    printf("Enter Key: ");
    scanf("%s", key);

    int dataLen = my_strlen(data);
    int keyLen = my_strlen(key);
    int n = keyLen - 1;

    my_strcpy(newData, data);
    for (int i = dataLen; i < dataLen + n; i++) newData[i] = '0';
    newData[dataLen + n] = '\0';

    printf("\nNew Data = %s\n", newData);

    int totalLen = my_strlen(newData);
    int pos = 0;

    while (pos <= totalLen - keyLen) {
        while (pos < totalLen && newData[pos] == '0') pos++;
        if (pos > totalLen - keyLen) break;

        printf("\nXOR starting at position %d\n", pos);
        for (int j = 0; j < keyLen; j++) {
            newData[pos + j] = (newData[pos + j] == key[j]) ? '0' : '1';
        }
        printf("After XOR = %s\n", newData);
        pos++;
    }

    for (int i = 0; i < n; i++) remainder[i] = newData[totalLen - n + i];
    remainder[n] = '\0';

    printf("\nRemainder = %s\n", remainder);

    my_strcpy(newData, data);
    for (int i = 0; i < n; i++) newData[dataLen + i] = remainder[i];
    newData[dataLen + n] = '\0';

    printf("Final Data = %s\n", newData);

    struct Packet packet;
    my_strcpy(packet.key, key);
    my_strcpy(packet.finalData, newData);

    int sent = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&receiver, sizeof(receiver));
    if (sent < 0) {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    printf("\nKey = %s\n", packet.key);
    printf("Final Data = %s\n", packet.finalData);

    close(sock);
    return 0;
}
