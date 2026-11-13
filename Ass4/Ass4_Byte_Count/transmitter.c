#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int len_function(char *data){
    int count = 0;
    while(data[count] != '\0'){
        count++;
    }
    return count;
}

int main()
{
    int sock;

    struct sockaddr_in receiver;
    char data[100][100];
    char res[100]="";
    int size=0;

    printf("Enter number of packets to transmit: ");
    scanf("%d",&size);

    for (int i=0;i<size;i++){
        scanf("%s",data[i]);
    }
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    char len[2];
    int len_int;
    for(int i=0;i<size;i++){
        len_int=len_function(data[i])+1;

        len[0]=(len_int)+'0';
        len[1]='\0';
        strcat(res,len);
        strcat(res,data[i]);
    }

    printf("%s",res);
    sock = socket(AF_INET,SOCK_DGRAM,0);

    sendto(sock,
           res,
           strlen(res)+1,
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));
        
    close(sock);

    return 0;
}