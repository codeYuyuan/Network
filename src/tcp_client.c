//
// Created by Yuyuan Liu on 2020-01-25.
//

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "tcp_common.h"
#define PORT_SIZE 16 
#define HOSTNAME_SIZE 64
#define USAGE "1. add [KEY] [VALUE] \n2. getvalue [KEY] \n3. getall \n4. remove [KEY]\n5. quit"
#define TOKENS 8 //as getValue has 7 characters


enum cmd_type _get_type(char *type) {
    if (strncmp("add", type, TOKENS) == 0) {
        return ADD;
    }
    else if (strncmp("getvalue", type, TOKENS) == 0) {
        return GETVALUE;
    }
    else if (strncmp("getall", type, TOKENS) == 0) {
        return GETALL;
    }
    else if (strncmp("remove", type, TOKENS) == 0) {
        return REMOVE;
    }
    else if (strncmp("quit", type, TOKENS) == 0) {
        return QUIT;
    }
    else {
        return INVALID;
    }
}


int parse_cmd(char *buf, struct command *cmd) {
    char type[8] = { '\0' };
    int toks = 0;

    toks = sscanf(buf, " %s %s %s ", type, cmd->key, cmd->value);
    if (toks > 0) { // Well formed
        cmd->key[MAX_KEY_SIZE - 1] = '\0';
        cmd->value[MAX_VALUE_SIZE - 1] = '\0';
    }
    else { // Invalid
        fprintf(stderr, "sscanf failed to scan input.\n");
        return -1;
    }

    cmd->type = _get_type(type);
    cmd->errorCode = 0;

    return 0;
}

char * get_input(char *buf) {
    memset(buf, '\0', CMD_SIZE);

    // get input
    if (NULL == fgets(buf, CMD_SIZE, stdin)) {
        fprintf(stderr, "fgets failed.\n");
        return NULL;
    }
    // remove newline
    int newline_pos = strcspn(buf, "\n");
    buf[newline_pos] = '\0';

    return buf;
}



int _add(int sockfd, struct command *cmd) {

    // Send put request
    send_cmd(sockfd, cmd);
    // Recieve handshake
    if (-1 == recv_cmd(sockfd, cmd)) {
        fprintf(stderr, "Error: client: failed to receive handshake command.\n");
        cmd->errorCode = -1;
        return -1;
    }

    return 0;
}

int _get(int sockfd, struct command *cmd) {

    // send get request
    send_cmd(sockfd, cmd);

    // Recieve handshake
    if (-1 == recv_cmd(sockfd, cmd)) {
        fprintf(stderr, "Error: client: failed to receive handshake command.\n");
        cmd->errorCode = -1;
        return -1;
    } else{
        printf("Received value %s from key %s\n", cmd->value, cmd->key);
        return 0;
    }
}

int _remove(int sockfd, struct command *cmd) {

    // send get request
    send_cmd(sockfd, cmd);

    // Recieve handshake
    if (-1 == recv_cmd(sockfd, cmd)) {
        fprintf(stderr, "Error: client: failed to receive handshake command.\n");
        cmd->errorCode = -1;
        return -1;
    } else if(cmd->errorCode == 1){
        printf("Successfully removed key  %s\n", cmd->key);
        return 0;
    } else{
        printf("Fail to remove key  %s\n", cmd->key);
        return -1;
    }
}

int _get_all(int sockfd, struct command *cmd) {

    // send get request
    send_cmd(sockfd, cmd);

    // Recieve handshake
    if (-1 == recv_cmd(sockfd, cmd)) {
        fprintf(stderr, "Error: client: failed to receive handshake command.\n");
        cmd->errorCode = -1;
        return -1;
    } else{
        printf("All key values pairs: %s\n", cmd->value);
        return 0;
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];
    int status;
    char port[PORT_SIZE];
    char hostname[HOSTNAME_SIZE];
    char cmd_buf[CMD_SIZE];
    struct command cmd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (argc != 3) {
        printf("Usage: %s <host name> <port number>", argv[0]);
        exit(1);
    }
    else {
        strncpy(hostname, argv[1], strlen(argv[1]));
        hostname[strlen(argv[1])] = '\0';
        strncpy(port, argv[2], strlen(argv[2]));
        port[strlen(argv[2])] = '\0';
        printf("Using hostname %s and port %s\n", hostname, port);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s on socket %d\n", s, sockfd);

    freeaddrinfo(servinfo); // all done with this structure

    set_timeout(sockfd);

    while(1) {
        printf("Command supported:\n%s\n", USAGE);

        if (-1 == parse_cmd(get_input(cmd_buf), &cmd)) {
            fprintf(stderr, "Poorly formed command.  Try again.\n");
            continue;
        }

        if (cmd.type == INVALID) {
            fprintf(stderr, "Poorly formed command.  Try again.\n");
            continue;
        }
        else if (cmd.type == ADD) {
            if (0 != (status = _add(sockfd, &cmd))) {
                fprintf(stderr, "Error: client: failed to execute %u command: %d\n", cmd.type, status);
            }
        }
        else if (cmd.type == GETVALUE) {
            if (0 != (status = _get(sockfd, &cmd))) {
                fprintf(stderr, "Error: client: failed to execute %u command: %d\n", cmd.type, status);
            }
        }
        else if (cmd.type == GETALL) {
            if (0 != (status = _get_all(sockfd, &cmd))) {
                fprintf(stderr, "Error: client: failed to execute %u command: %d\n", cmd.type, status);
            }
        }
        else if (cmd.type == REMOVE) {
            if (0 != (status = _remove(sockfd, &cmd))) {
                fprintf(stderr, "Error: client: failed to execute %u command: %d\n", cmd.type, status);
            }
        }
        else if (cmd.type == QUIT) {
            printf("Client: closing connections with socket %d.\n", sockfd);
            send_cmd(sockfd, &cmd);
            break;
        }
        else {
            fprintf(stderr, "Invalid command option.  Try again.\n");
        }
    }

    close(sockfd);

    exit(EXIT_SUCCESS);
}
