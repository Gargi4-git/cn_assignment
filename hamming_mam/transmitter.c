#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Check power of 2 without using << */
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
    char data[200];
    char manip, parity;
    int manipIndex = 0;

    printf("Enter data bits: ");

    if (scanf("%199s", data) != 1)
        return 1;

    int m = strlen(data);

    if (m == 0)
    {
        printf("Data cannot be empty.\n");
        return 1;
    }

    for (int i = 0; i < m; i++)
    {
        if (data[i] != '0' && data[i] != '1')
        {
            printf("Enter only binary digits (0 and 1).\n");
            return 1;
        }
    }

    printf("Do you want to manipulate a bit in the final Hamming code? (y/n): ");
    scanf(" %c", &manip);

    if (manip == 'y' || manip == 'Y')
    {
        printf("Enter bit position to manipulate (1-indexed Hamming position): ");
        scanf("%d", &manipIndex);
    }

    printf("Choose parity method - even (e) or odd (o): ");
    scanf(" %c", &parity);

    if (parity != 'e' && parity != 'E' &&
        parity != 'o' && parity != 'O')
    {
        printf("Invalid parity choice.\n");
        return 1;
    }

    int isEven = (parity == 'e' || parity == 'E');

    int r = 0;
    int power = 1;

    while (power < m + r + 1)
    {
        r++;
        power = power * 2;
    }

    int n = m + r;

    if (n >= 200)
    {
        printf("Data is too long for the codeword buffer.\n");
        return 1;
    }

    if (manipIndex < 0 || manipIndex > n)
    {
        printf("Invalid manipulation position.\n");
        return 1;
    }

    printf("\nData                : %s\n", data);
    printf("Data length (m)     : %d\n", m);
    printf("Manipulate bit?     : %s",
           (manipIndex > 0) ? "Yes -> position " : "No\n");

    if (manipIndex > 0)
        printf("%d\n", manipIndex);

    printf("Parity method       : %s\n",
           isEven ? "Even" : "Odd");
    printf("Parity bits needed  : r = %d\n", r);
    printf("Total codeword len  : n = m + r = %d\n", n);

    int *code = calloc(n + 1, sizeof(int));

    if (code == NULL)
    {
        perror("Memory allocation failed");
        return 1;
    }

    int di = 0;

    printf("\nPlacing data bits into non-parity positions:\n");

    for (int i = n; i >= 1; i--)
    {
        if (!isPowerOf2(i))
        {
            code[i] = data[di] - '0';

            printf("  position %2d -> data bit d%d = %d\n",
                   i, di + 1, code[i]);

            di++;
        }
    }

    printf("\n");

    for (int i = 0; i < r; i++)
    {
        int p = 1;

        for (int j = 0; j < i; j++)
            p = p * 2;

        int x = 0;
        int ones = 0;

        printf("Computing P%d (position %d):\n", p, p);
        printf("  positions used -> ");

        for (int j = p; j <= n; j = j + 2 * p)
        {
            for (int k = j; k < j + p && k <= n; k++)
            {
                printf("%d ", k);
            }
        }

        printf("\n  bits at those positions (P%d itself counted as 0 for now) -> ", p);

        for (int j = p; j <= n; j = j + 2 * p)
        {
            for (int k = j; k < j + p && k <= n; k++)
            {
                printf("%d ", code[k]);

                x = XOR(x, code[k]);

                if (code[k] == 1)
                    ones++;
            }
        }

        printf("\n  number of 1's (excluding P%d) = %d\n", p, ones);

        if (isEven)
            code[p] = x;
        else
            code[p] = XOR(x, 1);

        printf("  -> P%d = %d  (%s parity chosen)\n\n",
               p, code[p], isEven ? "even" : "odd");
    }

    printf("Parity bits summary: ");

    for (int i = 0; i < r; i++)
    {
        int p = 1;

        for (int j = 0; j < i; j++)
            p = p * 2;

        printf("P%d=%d  ", p, code[p]);
    }

    printf("\n\n");

    printf("Final Hamming codeword (before any manipulation), position %d down to 1:\n  ",
           n);

    for (int i = n; i >= 1; i--)
        printf("%d", code[i]);

    printf("\n");

    if (manipIndex > 0)
    {
        printf("Manipulating bit at position %d: %d -> %d\n",
               manipIndex, code[manipIndex],
               XOR(code[manipIndex], 1));

        code[manipIndex] = XOR(code[manipIndex], 1);

        printf("Codeword after manipulation:\n  ");

        for (int i = n; i >= 1; i--)
            printf("%d", code[i]);

        printf("\n\n");
    }
    else
    {
        printf("No manipulation applied. Sending clean codeword.\n\n");
    }

    char codestr[200];
    int idx = 0;

    for (int i = n; i >= 1; i--)
        codestr[idx++] = code[i] + '0';

    codestr[n] = '\0';

    char buffer[300];

    snprintf(buffer, sizeof(buffer), "%c,%d,%s",
             parity, n, codestr);

    struct sockaddr_in receiver;

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9100);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        free(code);
        return 1;
    }

    if (sendto(sock, buffer, strlen(buffer), 0,
               (struct sockaddr *)&receiver,
               sizeof(receiver)) < 0)
    {
        perror("Send failed");
        free(code);
        close(sock);
        return 1;
    }

    printf("Sent to receiver -> %s\n", buffer);

    free(code);
    close(sock);

    return 0;
}