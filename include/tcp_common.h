//
// Created by Yuyuan Liu on 2020-01-25.
//

#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

#define CMD_SIZE (sizeof (struct command) + 4)
#define TIMEOUT 10
#define MAX_KEY_SIZE 20
#define MAX_VALUE_SIZE 1000


enum cmd_type {
    QUIT = 0,
    ADD = 1,
    GETVALUE = 2,
    GETALL = 3,
    REMOVE = 4,
    INVALID = 5
};

struct Entry {
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
};

struct command { 
    enum cmd_type type;
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
    int errorCode;
};


int recv_cmd(int sockfd, struct command *cmd);

void send_cmd(int sockfd, struct command *cmd);


void free_cmd(struct command *cmd);

void print_cmd(struct command *cmd);

void set_timeout(int sockfd);

void * get_in_addr(struct sockaddr *sa);

#endif
