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
    int sock;
    struct sockaddr_in server;
    char filename[100];
    char buffer[SIZE];
    int choice, n;
    FILE *fp;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Select file type:\n");
    printf("1. Text file\n");
    printf("2. Audio file (MP3)\n");
    printf("3. Video file (MP4)\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            strcpy(filename, "text.txt");
            break;
        case 2:
            strcpy(filename, "audio.mp3");
            break;
        case 3:
            strcpy(filename, "video.mp4");
            break;
        default:
            printf("Invalid choice\n");
            close(sock);
            return 1;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File opening failed");
        close(sock);
        return 1;
    }

    /* Send the selected file type as one byte */
    char type = (char)choice;
    if (send(sock, &type, 1, 0) != 1) {
        perror("Sending file type failed");
        fclose(fp);
        close(sock);
        return 1;
    }

    /* Send the file contents */
    while ((n = fread(buffer, 1, SIZE, fp)) > 0) {
        int sent = 0;

        while (sent < n) {
            int bytes = send(sock, buffer + sent,
                             n - sent, 0);
            if (bytes <= 0) {
                perror("Sending file failed");
                fclose(fp);
                close(sock);
                return 1;
            }
            sent += bytes;
        }
    }

    if (ferror(fp)) {
        perror("File reading failed");
    } else {
        printf("%s sent successfully\n", filename);
    }

    fclose(fp);

    /* Tell the receiver no more bytes will be sent */
    shutdown(sock, SHUT_WR);
    close(sock);

    return 0;
}