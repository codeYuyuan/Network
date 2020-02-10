//
// Created by Yuyuan Liu on 2020-02-07.
// NSID: yul556
// Student ID: 11211784
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAXDATASIZE 128

int window_size; // go back n window size
int current_size;
char *hostname;
int portno;
int timeout;

struct my_thread_params
{
    int *keys;
    char **values;
    int *in_use;
    int *id;
    int *send_id;
};

int add_into_queue(int keys[], char *values[], int in_use[], int *id, char *value){
    if (current_size == window_size){
        printf("Queue full\n");
        return -1;
    }
    for (int i = 0; i < window_size; i++){
        if (in_use[i] == 0){
            keys[i] = *id;
            values[i] = (char *)malloc(strlen(value));
            memcpy(values[i], value, strlen(value));
            in_use[i] = 1;
            current_size += 1;
            printf("Adding ID %d into queue, current queue size is %d\n", *id, current_size);
            *id += 1;
            return 0;
        }
    }
    return -1;
}

int getIndex(int keys[], int in_use[], int key){
    for (int i = 0; i < window_size; i++){
        if (in_use[i] == 1 && key == keys[i]){
            return i;
        }
    }
    return -1;
}

void* send_packet_thread(void *ptr){
    struct my_thread_params *args = (struct my_thread_params *)ptr;
    int *keys = args->keys;
    char **values = args->values;
    int *in_use = args->in_use;
    int send_id = *(args->send_id);

    int sockfd;
    socklen_t serverlen;
    struct sockaddr_in serveraddr;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("Error in opening socket");
        exit(-1);
    }

    // set socket time out
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    struct hostent *server;
    server = gethostbyname(hostname);
    // config server addr
    memset((char *)&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memcpy((char *)&serveraddr.sin_addr.s_addr,(char *)server->h_addr,
           server->h_length);
    serveraddr.sin_port = htons(portno);

    serverlen = sizeof(serveraddr);

    char send_message[MAXDATASIZE];
    char receive_message[MAXDATASIZE];
    char temp[MAXDATASIZE];

    memset(send_message, 0, sizeof(send_message));
    memset(receive_message, 0, sizeof(receive_message));
    memset(temp, 0, sizeof(temp));

    int index = getIndex(keys, in_use, send_id);
    // simulate a header containing packet ID
    memcpy(temp, values[index], strlen(values[index]));
    snprintf(send_message, sizeof(send_message), "%d", send_id);
    strcat(send_message, ":");
    strcat(send_message, temp);

    int send_size = sendto(sockfd, send_message, sizeof(send_message), 0, (struct sockaddr *)&serveraddr, serverlen);
    printf("send %d byte of data (ID:Value) : %s\n", send_size, send_message);

    int recv_size = recvfrom(sockfd, receive_message, sizeof(receive_message), 0, (struct sockaddr *)&serveraddr, &serverlen);
    if(recv_size >= 0){
        printf("receive %d byte of data with ID: %s\n", recv_size, receive_message);
        in_use[index] = 0;
        current_size--;
    }else{
        // current id didn't get acknowledged, resend packet back n 
        pthread_t pid;

        // known bug here, should be go back n, but met a segment fault 
        /*
        if( send_id - window_size < 0){
            int* start = (int *)malloc(sizeof(int)); 
            *start = 0;
            args->send_id = start;
        }else{
            int go_back_n = send_id - window_size;
            int* start = (int *)malloc(sizeof(int)); 
            *start = go_back_n;
            args->send_id = start;
        }
        */
        pthread_create(&pid, NULL, send_packet_thread, args);
        pthread_join(pid, NULL);
    }
    pthread_exit(NULL);
}


int main(int argc, char **argv){
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <HOSTNAME> <PORT> <WINDOW SIZE> <TIMEOUT(second)>\n", argv[0]);
        exit(0);
    }
    
    // parse arguments
    hostname = argv[1];
    struct hostent *server;
    server = gethostbyname(hostname);
    if (server == NULL){
        fprintf(stderr, "Error when finding server %s\n", hostname);
        exit(-1);
    }
    portno = atoi(argv[2]);
    window_size = atoi(argv[3]);
    timeout = atoi(argv[4]);
    int keys[2 * window_size];
    char *values[2 * window_size];
    int in_use[2 * window_size];
    int id = 0;
    current_size = 0;

    struct my_thread_params args;
    args.keys = keys;
    args.values = values;
    args.in_use = in_use;
    args.id = &id;

    char message[MAXDATASIZE];
    
    while (1){
        if (current_size == window_size){
            // Queue Full;
            continue;
        }
        else{
            memset(message, 0, sizeof(message));
            printf("Enter the message:");
            int i = 0;
            while ((message[i++] = getchar()) != '\n');
            add_into_queue(keys, values, in_use, &id, message);

            int prev_id = id -1;
            args.send_id = &prev_id;
            pthread_t pid;
            pthread_create(&pid, NULL, send_packet_thread, (void *)&args);
            pthread_join(pid,NULL);
        }
    }
    return 0;
}
