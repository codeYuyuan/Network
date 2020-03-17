
// Name: Yuyuan Liu
// NSID: yul556
// Student ID: 11211784

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcp_util.h"

#define PORT_SIZE 16
#define HOSTNAME_SIZE 64
#define BACKLOG 10
#define HOSTNAME_LEN 256
#define NETWORK_SIZE 26

char hostname[HOSTNAME_LEN];

int dist[NETWORK_SIZE];
int matrix[NETWORK_SIZE][NETWORK_SIZE];
char next_hops[NETWORK_SIZE];


int my_id;
char my_tag;
char* my_port;



int get_id(char tag){
    return tag - 'a';
}


// helper function to parse int to string
char* itoa(int val){
    int base = 10;
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}

// connect neighbor router with given on port,return the sockfd for the connections.

int connect_neighbors(int port){
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if ((status = getaddrinfo(hostname, itoa(port), &hints, &servinfo)) != 0) {
        fprintf(stderr, "tcp getaddrinfo: %s\n", gai_strerror(status));
        return -1;
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
        fprintf(stderr, "client: failed to connect port %d \n", port);
        return -1;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s on socket %d\n", s, sockfd);

    freeaddrinfo(servinfo); // all done with this structure

    set_timeout(sockfd);
    return sockfd;
}


// helper function to concatenate string
char *concatenate(size_t size, char *array[size], const char *joint){
    size_t jlen, lens[size];
    size_t i, total_size = (size-1) * (jlen=strlen(joint)) + 1;
    char *result, *p;


    for(i=0;i<size;++i){
        total_size += (lens[i]=strlen(array[i]));
    }
    p = result = malloc(total_size);
    for(i=0;i<size;++i){
        memcpy(p, array[i], lens[i]);
        p += lens[i];
        if(i<size-1){
            memcpy(p, joint, jlen);
            p += jlen;
        }
    }
    *p = '\0';
    return result;
}


// sending routing table to neighbors
int send_rumors(int argc, int ports[]){
    // connect neighbors and send distance map
    for(int i = 0 ; i <= argc - 3; i ++){
        int sockfd = connect_neighbors(ports[i]);
        struct command* cmd = (struct command*)malloc(sizeof(struct command));
        cmd->router_name = my_tag;
        cmd->port = atoi(my_port);
        char* c_array[NETWORK_SIZE];
        for(int i = 0 ; i < NETWORK_SIZE; i++){
            char c[sizeof(int)];
            snprintf(c, sizeof(int), "%d", dist[i]);
            c_array[i] = malloc(sizeof(c));
            strcpy(c_array[i], c);
        }

        char* res = concatenate(NETWORK_SIZE, c_array, ",");
        memset(cmd->distance, '0', sizeof(char) * NETWORK_SIZE * 3 + 1);
        for(int i = 0 ; i < sizeof(cmd->distance) / sizeof(char); i++){
            cmd->distance[i] = res[i];
        }
        cmd->distance[NETWORK_SIZE * 3] = '\0';
        cmd->error_code = 0;
        if(sockfd > 0){
            send_cmd(sockfd, cmd);
            return 1;
        }
    }
    return -1;
}

void parseDistance(char* raw_data, int* distance){
    char delim[] = ",";
    char *ptr = strtok(raw_data, delim);
    int i = 0;
    while (ptr != NULL) {
        distance[i++] = atoi(ptr);
        ptr = strtok(NULL, delim);
    }
}

// check if the given port is a neighbor router, if it is, return 1 and set the dis to 1, otherwise return -1;
int isNeighbor(int* ports, int port, int num_of_ports, char router_name){
    for(int i = 0 ; i < num_of_ports; i++){
        if(ports[i] == port){
            int id = get_id(router_name);
            dist[id] = 1;
            return 1;
        }
    }
    return -1;
}


void print_routing_table(){
    printf("========Routing table of router %c=========\n", my_tag);
    for(int i = 0 ; i < NETWORK_SIZE; i++){
        printf("Router %c -> %c : %3d | next_hop : %c \n", my_tag, 'a' + i, dist[i], next_hops[i] ? next_hops[i] : '-');
    }
}

void init_routing_table(){
    for(int i = 0; i < NETWORK_SIZE; i++){
        if(i == get_id(my_tag)){
            dist[i] = 0;
        }else{
            dist[i] = 99;
        }
        matrix[i][i] = 0;
    }
}


/**
 * From Beej's Guide to Network Programming.
 */
int main(int argc, char **argv) {


    int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int status;

    int ports[argc];
    int j = 0;

    my_tag = argv[1][0];
    my_id = get_id(my_tag);
    printf("Current router name: %c\n", my_tag);
    my_port = argv[2];
    for(int i = 3; i < argc; i++){
        ports[j++] = atoi(argv[i]);
        if(ports[j-1] < 30000 || ports[j-1]> 40000){
            perror("Port needs to be within 30000 to 40000\n");
        }
        printf("Neighbor router %d on port: %d\n", j,ports[j-1]);
    }

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((status = getaddrinfo(NULL, my_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

    gethostname(hostname, HOSTNAME_LEN);
    printf("Router started at %s:%s\n", hostname, my_port);

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


    // initialize routing table
    init_routing_table();
    print_routing_table();

    int pid;
    while(1) { // main loop to update distance
        send_rumors(argc, ports);
        sin_size = sizeof(their_addr);
        printf("Server: Waiting for connections...\n");
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        pid = fork();
        if (pid < 0) {
            perror("ERROR in new process creation");
        }
        else{
            // should put the single client handing part here, but i was running into problems that this while loops creates too many
            // process.
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("Server: Waiting for updates...\n");
        struct command* cmd = (struct command*) malloc(sizeof(struct command));

        if (-1 == recv_cmd(new_fd, cmd)) {
            fprintf(stderr, "Error: server: failed to receive data.\n");
            sleep(2);
            continue;
        }else{
            char neighbor_name = cmd -> router_name;
            int neighbor_id = get_id(neighbor_name);
            parseDistance(cmd -> distance, matrix[neighbor_id]);
            dist[neighbor_id] = 1;

            // updating using Bellman Ford's Algorithm
            for(int i = 0 ; i < NETWORK_SIZE; i++){
                if(i != my_id && dist[i] > dist[neighbor_id] + matrix[neighbor_id][i]){
                    dist[i] = dist[neighbor_id] + matrix[neighbor_id][i];
                    next_hops[i] = neighbor_name;
                }
            }
        }

        print_routing_table();
        sleep(2);

    }
}
