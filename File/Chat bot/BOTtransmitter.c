
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
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
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
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP address\n");
        close(sockfd);
        return 1;
    }

    printf("UDP Chat Client started.\n");
    printf("Type messages and press Enter. Type 'exit' to quit.\n");

    while (1) {
        // Enter client message
        printf("You: ");
        if (fgets(buffer, SIZE, stdin) == NULL)
            break;

        buffer[strcspn(buffer, "\n")] = '\0';

        // Send message to server
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server, len) < 0) {
            perror("Send failed");
            break;
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Chat ended.\n");
            break;
        }

        // Receive server reply
        n = recvfrom(sockfd, buffer, SIZE - 1, 0, NULL, NULL);

        if (n < 0) {
            perror("Receive failed");
            break;
        }

        buffer[n] = '\0';

        printf("Server: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Server ended the chat.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
