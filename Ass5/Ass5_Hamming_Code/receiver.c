    #include<stdio.h>
    #include<stdlib.h>
    #include<string.h>
    #include<unistd.h>
    #include<sys/socket.h>
    #include<netinet/in.h>
    #include<arpa/inet.h>

    int isPowerOf2(int x)
    {
        return (x & (x-1)) == 0;
    }

    int main()
    {
        struct sockaddr_in receiver, sender;
        receiver.sin_family = AF_INET;
        receiver.sin_port = htons(9100);
        receiver.sin_addr.s_addr = INADDR_ANY;

        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        bind(sock, (struct sockaddr*)&receiver, sizeof(receiver));

        socklen_t len = sizeof(sender);
        char buffer[300];

        printf("Waiting for Hamming codeword...\n\n");
        int rcv = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&sender, &len);
        buffer[rcv] = '\0';

        char parity;
        int n;
        char codestr[200];
        sscanf(buffer, "%c,%d,%s", &parity, &n, codestr);
        int isEven = (parity == 'e' || parity == 'E');

        printf("Received raw message : %s\n", buffer);
        printf("Parity method        : %s\n", isEven ? "Even" : "Odd");
        printf("Codeword length (n)  : %d\n", n);
        printf("Received codeword    : %s  (position %d down to 1, left to right)\n\n", codestr, n);

        int *code = calloc(n + 1, sizeof(int));
        int idx = 0;
        for(int i = n; i >= 1; i--) code[i] = codestr[idx++] - '0';

        int r = 0;
        while((1 << r) <= n) r++;

        int syndrome = 0;
        printf("Recomputing parity checks:\n");
        for(int i = 0; i < r; i++)
        {
            int p = 1 << i;
            if(p > n) break;
            int x = 0;
            for(int j = p; j <= n; j += 2*p)
            {
                for(int k = j; k < j+p && k <= n; k++)
                {
                    x ^= code[k];
                }
            }
            int mismatch = isEven ? x : (x ^ 1);
            printf("  Check C%d (position %d): mismatch = %d\n", p, p, mismatch);
            syndrome += mismatch * p;
        }

        printf("\nBinary error position = %d\n", syndrome);

        if(syndrome == 0)
        {
            printf("No error detected. Codeword is correct.\n\n");
        }
        else
        {
            printf("Error detected at position %d -> flipping it back (%d -> %d)\n\n",
                syndrome, code[syndrome], code[syndrome]^1);
            code[syndrome] ^= 1;
        }

        printf("Corrected Hamming codeword (position %d down to 1): ", n);
        for(int i = n; i >= 1; i--) printf("%d", code[i]);
        printf("\n");

        printf("Extracted actual data     : ");
        for(int i = n; i >= 1; i--)
        {
            if(!isPowerOf2(i)) printf("%d", code[i]);
        }
        printf("\n");

        free(code);
        close(sock);
        return 0;
    }