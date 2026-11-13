#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define SIZE 1024

int main() {
    int sockfd;
    char buffer[SIZE];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    ssize_t n;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("UDP Chat Server started on port %d\n", PORT);
    printf("Waiting for client messages...\n");

    while (1) {
        // Receive message from client
        n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                     (struct sockaddr *)&client, &len);

        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';

        printf("\nClient: %s\n", buffer);

        // Exit if client sends exit
        if (strcmp(buffer, "exit") == 0) {
            printf("Client ended the chat.\n");
            break;
        }

        // Enter server reply
        printf("You: ");
        if (fgets(buffer, SIZE, stdin) == NULL)
            break;

        buffer[strcspn(buffer, "\n")] = '\0';

        // Send reply to client
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&client, len) < 0) {
            perror("Send failed");
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Chat ended.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
