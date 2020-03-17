
// Name: Yuyuan Liu
// NSID: yul556
// Student ID: 11211784

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "tcp_util.h"


int _deserialize_cmd(char *cmd_buf, struct command *cmd) {
    memset(cmd->distance, 0, 26 * 3 + 1);
    if (sscanf(cmd_buf, "%c %d %s %d",
                &(cmd->router_name), &(cmd->port), cmd->distance, &(cmd->error_code)) < 0) {
        fprintf(stderr, "sscanf failed to scan input.\n");
        return -1;
    }

    return 0;
}


int _serialize_cmd(char *buf, struct command *cmd) {
    return sprintf(buf, "%c %d %s %d",
            cmd->router_name, cmd->port, cmd->distance, cmd->error_code);
}

int recv_cmd(int sockfd, struct command *cmd) {
    char cmd_buf[CMD_SIZE] = { '\0' };

    if (-1 == (recv(sockfd, cmd_buf, CMD_SIZE, 0))) {
        perror("recv");
        return -1;
    }

    printf("[%d] received serialized data '%s' on socket %d\n", getpid(), cmd_buf, sockfd);

    if (-1 == (_deserialize_cmd(cmd_buf, cmd))) {
        fprintf(stderr, "Error: Process %d failed to deserialize the command.\n", getpid());
        return -1;
    }

    return 0;
}

void send_cmd(int sockfd, struct command *cmd) {
    char *cmd_buf = malloc(sizeof(struct command));
    _serialize_cmd(cmd_buf, cmd);
    printf("[%d] sent serialized data '%s' on socket %d\n", getpid(), cmd_buf, sockfd);
    send(sockfd, cmd_buf, CMD_SIZE, 0);
}


void print_cmd(struct command *cmd) {
    if (NULL == cmd) {
        printf("NULL command.");
    }
    else {
        printf("router name: %c, port: %d, distance: %s, error_code: %d\n",
                cmd->router_name, cmd->port, cmd->distance, cmd->error_code);
    }
}

void set_timeout(int sockfd) {
    struct timeval tv;
    // Set timeout
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

/**
 * From Beej's Guide
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) { // IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}
