//
// Created by Yuyuan Liu on 2020-02-07.
// NSID: yul556
// Student ID: 11211784
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOSTNAME_LEN 256
#define PORT 30001
#define MAXDATASIZE 128	 // max message length

int main(int argc, char **argv)
{
	char hostname[HOSTNAME_LEN];
	int sockfd;					  
	socklen_t client_len;		   
	struct sockaddr_in server_addr; 
	struct sockaddr_in client_addr;    
	char *hostaddrp;			   
	int reuse = 1;					   
	int current_seq_id = 0;
	char message[MAXDATASIZE * 2];
	char get_message[10];
	char send_message[MAXDATASIZE * 2];

	if(argc != 2){
		fprintf(stderr, "Usage: %s <Probability [0 - 100] for packet loss/corruption>\n", argv[0]);
        exit(0);
	}
	int p_loss = atoi(argv[1]);
	if(p_loss > 100){
		fprintf(stderr, "[Error] Illegal argument: Probability for packet loss should be within [0, 100] \n");
        exit(0);
	}
	
	/* initialize UDP socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0){
		perror("[Error] Server: fail to initialize socket");
		return -1;
	}

	reuse = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			   &reuse, sizeof(reuse));

	/*
	* build the server's Internet address
	*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	client_len = sizeof(struct sockaddr);


	if (bind(sockfd, (struct sockaddr *)&server_addr,
			 sizeof(server_addr)) < 0)
		perror("[Error] error on binding socket");

	gethostname(hostname, HOSTNAME_LEN);
    printf("Server started...\nHost: %s \nPort: %d\n", hostname, PORT);

	
	while (1)
	{
		bzero(message, sizeof(message));
		bzero(get_message, sizeof(get_message));
		bzero(send_message, sizeof(send_message));

		recvfrom(sockfd, message, sizeof(message), 0,
							 (struct sockaddr *)&client_addr, &client_len);
		
		//get sender address 
		hostaddrp = inet_ntoa(client_addr.sin_addr);
		if (hostaddrp == NULL){
			perror("[Error] faile to get sender address via inet_ntoa\n");
		}

		char *ptr = strtok(message, ":");

		char *splitedMessage[2];
		int n = 0;
		while (ptr != NULL)
		{
			splitedMessage[n++] = ptr;
			ptr = strtok(NULL, ":");
		}

		int id = atoi(splitedMessage[0]);
		
		if (id != current_seq_id){
			printf("[Error] Received sequence ID out of order");
		}

		printf("Receive message id: %d, message: %s", id, splitedMessage[1]);

		int i = 0;
		printf("New message coming, acknowledge? [Y/N]: ");
		while ((get_message[i++] = getchar()) != '\n');

		if (strncmp(get_message, "Y", 1) == 0){
			// simulate packet loss with random
			int num = rand() % 100;
			if (num >= p_loss){ 
				current_seq_id += 1;
				snprintf(send_message, sizeof(send_message), "%d", id);
				sendto(sockfd, send_message, strlen(send_message), 0,
									(struct sockaddr *)&client_addr, client_len);
			}else{
				printf("Packet loss\n");
			}
		}else{
			printf("Decline incoming packet\n");
		}		
	}
	return 0;
}
