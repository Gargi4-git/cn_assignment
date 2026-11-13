#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<math.h>

int isPowerOf2(int x)
{
    return (x%2==0 && x%4==0);
}

int main()
{
    char data[200];
    char manip, parity;
    int manipIndex = 0;

    printf("Enter data bits: ");
    scanf("%s", data);
    int m = strlen(data);

    printf("Do you want to manipulate a bit in the final Hamming code? (y/n): ");
    scanf(" %c", &manip);
    if(manip == 'y' || manip == 'Y')
    {
        printf("Enter bit position to manipulate (1-indexed Hamming position): ");
        scanf("%d", &manipIndex);
    }

    printf("Choose parity method - even (e) or odd (o): ");
    scanf(" %c", &parity);
    int isEven = (parity == 'e' || parity == 'E');

    int r = 0;

    while(pow(2,r)<(m+r+1)){
        r++;
    }
    int n = m + r;

    printf("Data                : %s\n", data);
    printf("Data length (m)     : %d\n", m);
    printf("Manipulate bit?     : %s", (manipIndex > 0) ? "Yes -> position " : "No\n");
    if(manipIndex > 0) printf("%d\n", manipIndex);
    printf("Parity method       : %s\n", isEven ? "Even" : "Odd");
    printf("Parity bits needed  : r = %d\n", r);
    printf("Total codeword len  : n = m + r = %d\n", n);

    int *code = calloc(n + 1, sizeof(int));

    int di = 0;
    printf("Placing data bits into non-parity positions:\n");
    for(int i = n; i >= 1; i--)
    {
        if(!isPowerOf2(i))
        {
            code[i] = data[di] - '0';
            printf("  position %2d -> data bit d%d = %d\n", i, di+1, code[i]);
            di++;
        }
    }
    printf("\n");

    for(int i = 0; i < r; i++)
    {
        int p = 1 << i;
        int x = 0;
        printf("Computing P%d (position %d):\n", p, p);
        printf("  positions used -> ");
        for(int j = p; j <= n; j += 2*p)
        {
            for(int k = j; k < j+p && k <= n; k++)
            {
                printf("%d ", k);
            }
        }
        printf("\n  bits at those positions (P%d itself counted as 0 for now) -> ", p);
        int ones = 0;
        for(int j = p; j <= n; j += 2*p)
        {
            for(int k = j; k < j+p && k <= n; k++)
            {
                printf("%d ", code[k]);
                x ^= code[k];
                if(code[k]) ones++;
            }
        }
        printf("\n  number of 1's (excluding P%d) = %d\n", p, ones);

        code[p] = isEven ? x : (x ^ 1);
        printf("  -> P%d = %d  (%s parity chosen)\n\n", p, code[p], isEven ? "even" : "odd");
    }

    printf("Parity bits summary: ");
    for(int i = 0; i < r; i++) printf("P%d=%d  ", 1<<i, code[1<<i]);
    printf("\n\n");

    printf("Final Hamming codeword (before any manipulation), position %d down to 1:\n  ", n);
    for(int i = n; i >= 1; i--) printf("%d", code[i]);

    if(manipIndex > 0 && manipIndex <= n)
    {
        printf("Manipulating bit at position %d: %d -> %d\n", manipIndex, code[manipIndex], code[manipIndex]^1);
        code[manipIndex] ^= 1;
        printf("Codeword after manipulation:\n  ");
        for(int i = n; i >= 1; i--) printf("%d", code[i]);
        printf("\n\n");
    }
    else
    {
        printf("No manipulation applied. Sending clean codeword.\n\n");
    }

    char codestr[200];
    int idx = 0;
    for(int i = n; i >= 1; i--) codestr[idx++] = code[i] + '0';
    codestr[n] = '\0';

    char buffer[300];
    snprintf(buffer, sizeof(buffer), "%c,%d,%s", parity, n, codestr);

    struct sockaddr_in receiver;
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9100);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&receiver, sizeof(receiver));

    printf("Sent to receiver -> %s\n", buffer);

    free(code);
    close(sock);
    return 0;
}