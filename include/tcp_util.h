
// Name: Yuyuan Liu
// NSID: yul556
// Student ID: 11211784

#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>

#define CMD_SIZE (sizeof (struct command) + 4)
#define TIMEOUT 10
#define NETWORK_SIZE 26


struct command { 
    char router_name;
    int port;
    char distance[NETWORK_SIZE*3 + 1];
    int error_code;
};


int recv_cmd(int sockfd, struct command *cmd);

void send_cmd(int sockfd, struct command *cmd);

void print_cmd(struct command *cmd);

void set_timeout(int sockfd);

void * get_in_addr(struct sockaddr *sa);

#endif
