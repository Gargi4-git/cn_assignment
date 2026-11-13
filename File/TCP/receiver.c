#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 1024

int main()
{
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    char filename[100];
    char buffer[SIZE];
    char type;
    int n;
    FILE *fp;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        close(server_sock);
        return 1;
    }

    printf("Receiver waiting for sender...\n");

    client_sock = accept(server_sock,
                         (struct sockaddr *)&client,
                         &client_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        return 1;
    }

    printf("Sender connected\n");

    /* Receive the selected file type */
    n = recv(client_sock, &type, 1, MSG_WAITALL);
    if (n != 1) {
        printf("Could not receive file type\n");
        close(client_sock);
        close(server_sock);
        return 1;
    }

    switch (type) {
        case 1:
            strcpy(filename, "received.txt");
            break;
        case 2:
            strcpy(filename, "received.mp3");
            break;
        case 3:
            strcpy(filename, "received.mp4");
            break;
        default:
            printf("Invalid file type received\n");
            close(client_sock);
            close(server_sock);
            return 1;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Output file opening failed");
        close(client_sock);
        close(server_sock);
        return 1;
    }

    /* Receive and save the file contents */
    while ((n = recv(client_sock, buffer, SIZE, 0)) > 0) {
        if (fwrite(buffer, 1, n, fp) != (size_t)n) {
            perror("Writing file failed");
            fclose(fp);
            close(client_sock);
            close(server_sock);
            return 1;
        }
    }

    if (n < 0) {
        perror("Receiving file failed");
    } else {
        printf("File received successfully: %s\n", filename);
    }

    fclose(fp);
    close(client_sock);
    close(server_sock);

    return 0;
}