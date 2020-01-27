//
// Created by Yuyuan Liu on 2020-01-26.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUF 256
#define HOSTNAME_LEN 256
#define PORT 30001;


int main(int argc, char const *argv[]){
    int s = 0;
    int n = 0;
    int reuse = 1;
    int port = PORT;
    int cli_len = sizeof(struct sockaddr);
    char buf[MAXBUF] = {0};
    struct sockaddr_in addr, cli;
    char hostname[HOSTNAME_LEN];

    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    gethostname(hostname, HOSTNAME_LEN);

    /* initialize UDP socket*/
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s<0){
        perror("socket");
        return -1;
    }
    /* reuse socket*/
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
    printf("Server started...\nHost: %s \nPort: %d\n", hostname, port);

    while(1){
        memset(buf, 0, MAXBUF);
        /*read from socket*/
        n = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *)&cli, (socklen_t *)&cli_len);
        if(n<0){
            perror("recvfrom");
            return -1;
        }else{
            printf("receive message from %s(port=%d) len %d: %s\n",inet_ntoa(cli.sin_addr), port, n, buf);
        }
    }
    return 0;
}
