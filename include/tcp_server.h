//
// Created by Yuyuan Liu on 2020-01-25.
//


#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int get(char *local_file_name, char *remote_file_name);

int put(char *local_file_name, char *remote_file_name);

void quit(void);

#endif
