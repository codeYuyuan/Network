########################################
# OS and architecture macros
CURR_OS := $(shell uname -s)
ARCH := $(shell uname -m)
MAC_OS="Darwin"
LINUX_OS="Linux"
########################################
TARGET = tcp_server tcp_client tcp_proxy udp_server
########################################
# Directories
OBJ = ./obj/
INC = ./include/
SRC = ./src/
SRV_DIR = ./server_dir/
CLI_DIR = ./client_dir/
PRX_DIR = ./proxy_dir/
UDP_DIR = ./udp_dir/
$(shell mkdir -p $(OBJ))
$(shell mkdir -p $(SRV_DIR))
$(shell mkdir -p $(CLI_DIR))
$(shell mkdir -p $(PRX_DIR))
$(shell mkdir -p $(UDP_DIR))
########################################
# Compiler and linker options
CC = gcc
AR_OPTIONS = cr
C_FLAGS = -Wall -pedantic -g
INC_FLAGS = -I$(INC)
########################################

# Filename macroes
# server 
SERVER_H = $(INC)tcp_server.h
SERVER_OBJ = $(OBJ)tcp_server.o 
CLIENT_H = $(INC)tcp_client.h
CLIENT_OBJ = $(OBJ)tcp_client.o 
COMMON_H = $(INC)tcp_common.h
COMMON_OBJ = $(OBJ)tcp_common.o
PROX_OBJ = $(OBJ)tcp_proxy.o
UDP_SERVER_OBJ = $(OBJ)udp_server.o
# all
ALL_OBJ = $(CLIENT_OBJ) $(SERVER_OBJ) $(COMMON_OBJ) $(PROX_OBJ) $(UDP_SERVER_OBJ)
ALL_H = $(CLIENT_H) $(SERVER_H) $(COMMON_H)
EXEC = $(SRV_DIR)tcp_server $(CLI_DIR)tcp_client $(PRX_DIR)tcp_proxy $(UDP_DIR)udp_server
########################################
# Recipes
.PHONY: server all clean

all: $(TARGET)

# PROXY 
tcp_proxy : $(PROX_OBJ) $(COMMON_OBJ)
	$(CC) $^ -o $(PRX_DIR)$@
# SERVER 
tcp_server : $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $^ -o $(SRV_DIR)$@
# CLIENT
tcp_client : $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $^ -o $(CLI_DIR)$@
# UDPSERVER
udp_server : $(UDP_SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $^ -o $(UDP_DIR)$@


# PROXY OBJ FILES
$(PROX_OBJ) : $(SRC)tcp_proxy.c $(COMMON_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@
# SERVER OBJ FILES
$(SERVER_OBJ) : $(SRC)tcp_server.c $(SERVER_H) $(COMMON_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@
# CLIENT OBJ FILES
$(CLIENT_OBJ) : $(SRC)tcp_client.c $(CLIENT_H) $(COMMON_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@
# COMMON OBJ FILES
$(COMMON_OBJ) : $(SRC)tcp_common.c $(COMMON_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@
# UDP OBJ FILES
$(UDP_SERVER_OBJ) : $(SRC)udp_server.c $(COMMON_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@

clean:
	rm -f $(ALL_OBJ)
	rm -f $(EXEC)
	rmdir $(OBJ)
	rmdir $(SRV_DIR)
	rmdir $(CLI_DIR)
	rmdir $(PRX_DIR)
	rmdir $(UDP_DIR)
