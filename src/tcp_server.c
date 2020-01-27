//
// Created by Yuyuan Liu on 2020-01-25.
//

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcp_server.h"
#include "tcp_common.h"

#define PORT "30000"
#define BACKLOG 10
#define HOSTNAME_LEN 256
#define DBSIZE 20




struct Entry* db[DBSIZE];

/**
 * Returns  -1 If there is an error with the request
 *
 */
int _add(int sockfd, struct command *cmd) {
    char key[MAX_KEY_SIZE], value[MAX_VALUE_SIZE];
    strcpy(key, cmd->key);
    strcpy(value, cmd->value);
    int i = 0;
    for(; i < sizeof(db)/ sizeof(db[0]); i++){
        if(strcmp(db[i]->key, "") == 0){
            strcpy(db[i]->key, key);
            strcpy(db[i]->value, value);
            // Send confirmation command.
            printf("[INFO] (key: %s value: %s) added into DB\n", key, value);
            send_cmd(sockfd, cmd);
            return 0;
        }
    }
    if(i == sizeof(db)/ sizeof(db[0])){
        cmd->errorCode = -1;
        send_cmd(sockfd, cmd);
        return -1;
    }
    return -1;
}

/**
 * Returns  -1 If there is an error with the request
 *
 */
int _get(int sockfd, struct command *cmd) {
    char key[MAX_KEY_SIZE];
    strcpy(key, cmd->key);
    for(int i = 0 ; i < DBSIZE; i++){
        if(strcmp(db[i]->key, key) == 0){
            strcpy(cmd->value,db[i]->value);
            send_cmd(sockfd, cmd);
            return 0;
        }
    }
    return -1;
}

int _get_all(int sockfd, struct command *cmd) {
    char key[MAX_KEY_SIZE];
    strcpy(key, cmd->key);
    char* res = (char*) malloc(sizeof(char) * MAX_VALUE_SIZE);
    for(int i = 0 ; i < DBSIZE; i++){
        printf("out%d\n", i);
        if(strcmp(db[i]->key, "") != 0){
            char* entry;
            printf("%d\n", i);
            entry = (char*) malloc(sizeof(char) * 41);
            strcpy(entry, db[i]->key);
            strcat(entry, ":");
            strcat(entry, db[i]->value);
            strcat(entry, ";");
            strcat(res, entry);
            strcpy(cmd->value, res);
            send_cmd(sockfd, cmd);
            return 0;
        }
    }
    return -1;
}

/**
 * From Beej's Guide to Network Programming.
 */
int main(int argc, char **argv) {

    struct command cmd;
    char hostname[HOSTNAME_LEN];

    int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int status;


    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

    gethostname(hostname, HOSTNAME_LEN);
    printf("Server started...\nHost: %s \nPort: %s\n", hostname, PORT);

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sock_fd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1); }

		if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock_fd);
			perror("server: bind");
			continue;
		}

		break;
	}

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind to valid addrinfo");
        exit(1);
    }

    freeaddrinfo(servinfo); // all done with this structure


    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    for (int i = 0; i < DBSIZE; i++) {
        db[i] = (struct Entry*)malloc(sizeof(struct Entry));
    }


    while(1) { // main accept() loop
        sin_size = sizeof(their_addr);
        printf("Server: Waiting for connections...\n");
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("Server: Got a connection from %s\n", s);

        while(1) {
            printf("Server: Waiting for commands...\n");
            if (-1 == recv_cmd(new_fd, &cmd)) {
                fprintf(stderr, "Error: server: failed to receive command.\n");
                sleep(3);
                continue;
            }
            if (cmd.type == ADD) {
                if (0 != (status = _add(new_fd, &cmd))) {
                    fprintf(stderr, "Error: server: failed to execute %u command: %d\n", cmd.type, status);
                }
            }
            else if (cmd.type == GETVALUE) {
                if (0 != (status = _get(new_fd, &cmd))) {
                    fprintf(stderr, "Error: server: failed to execute %u command: %d\n", cmd.type, status);
                }
            }
            else if (cmd.type == GETALL) {
                if (0 != (status = _get_all(new_fd, &cmd))) {
                    fprintf(stderr, "Error: server: failed to execute %u command: %d\n", cmd.type, status);
                }
            }
            else if (cmd.type == QUIT) {
                printf("server: closing connections with socket %d.\n", new_fd);
                close(new_fd);
                break;
            }
            else {
               fprintf(stderr, "Invalid command format.\n");
            }
        }
    }
}
