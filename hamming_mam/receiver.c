#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int isPowerOf2(int x)
{
    if (x < 1)
        return 0;

    while (x % 2 == 0)
        x = x / 2;

    return x == 1;
}

/* XOR without using the ^ operator */
int XOR(int a, int b)
{
    if (a == b)
        return 0;
    else
        return 1;
}

int main()
{
    struct sockaddr_in receiver, sender;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9100);
    receiver.sin_addr.s_addr = INADDR_ANY;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&receiver,
             sizeof(receiver)) < 0)
    {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    socklen_t len = sizeof(sender);
    char buffer[300];

    printf("Waiting for Hamming codeword...\n\n");

    int rcv = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                       (struct sockaddr *)&sender, &len);

    if (rcv < 0)
    {
        perror("Receive failed");
        close(sock);
        return 1;
    }

    buffer[rcv] = '\0';

    char parity;
    int n;
    char codestr[200];

    if (sscanf(buffer, "%c,%d,%199s", &parity, &n, codestr) != 3)
    {
        printf("Invalid message received.\n");
        close(sock);
        return 1;
    }

    if (n < 1 || n >= 200 || (int)strlen(codestr) != n)
    {
        printf("Invalid codeword length.\n");
        close(sock);
        return 1;
    }

    int isEven = (parity == 'e' || parity == 'E');

    printf("Received raw message : %s\n", buffer);
    printf("Parity method        : %s\n",
           isEven ? "Even" : "Odd");
    printf("Codeword length (n)  : %d\n", n);
    printf("Received codeword    : %s\n\n", codestr);

    int *code = calloc(n + 1, sizeof(int));

    if (code == NULL)
    {
        perror("Memory allocation failed");
        close(sock);
        return 1;
    }

    int idx = 0;

    for (int i = n; i >= 1; i--)
    {
        if (codestr[idx] != '0' && codestr[idx] != '1')
        {
            printf("Invalid binary codeword.\n");
            free(code);
            close(sock);
            return 1;
        }

        code[i] = codestr[idx++] - '0';
    }

    int r = 0;
    int p = 1;

    while (p <= n)
    {
        r++;
        p = p * 2;
    }

    int syndrome = 0;

    printf("Recomputing parity checks:\n");

    for (int i = 0; i < r; i++)
    {
        p = 1;

        for (int j = 0; j < i; j++)
            p = p * 2;

        if (p > n)
            break;

        int x = 0;

        for (int j = p; j <= n; j = j + 2 * p)
        {
            for (int k = j; k < j + p && k <= n; k++)
            {
                x = XOR(x, code[k]);
            }
        }

        int mismatch;

        if (isEven)
            mismatch = x;
        else
            mismatch = XOR(x, 1);

        printf("  Check C%d (position %d): mismatch = %d\n",
               p, p, mismatch);

        syndrome = syndrome + mismatch * p;
    }

    printf("\nBinary error position = %d\n", syndrome);

    if (syndrome == 0)
    {
        printf("No error detected by parity checks.\n\n");
    }
    else if (syndrome <= n)
    {
        printf("Error detected at position %d -> flipping it back (%d -> %d)\n\n",
               syndrome, code[syndrome],
               XOR(code[syndrome], 1));

        code[syndrome] = XOR(code[syndrome], 1);
    }
    else
    {
        printf("Invalid syndrome. Cannot correct this position.\n");
        free(code);
        close(sock);
        return 1;
    }

    printf("Corrected Hamming codeword (position %d down to 1): ", n);

    for (int i = n; i >= 1; i--)
        printf("%d", code[i]);

    printf("\n");

    printf("Extracted actual data     : ");

    for (int i = n; i >= 1; i--)
    {
        if (!isPowerOf2(i))
            printf("%d", code[i]);
    }

    printf("\n");

    free(code);
    close(sock);

    return 0;
}