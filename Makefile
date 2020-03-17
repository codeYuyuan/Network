# Name: Yuyuan Liu
# NSID: yul556
# Student ID: 11211784

TARGET = tcp_router
# Directories
OBJ = ./obj/
INC = ./include/
SRC = ./src/
SRV_DIR = ./router_dir/
$(shell mkdir -p $(OBJ))
$(shell mkdir -p $(SRV_DIR))
########################################
# Compiler and linker options
CC = gcc
AR_OPTIONS = cr
C_FLAGS = -Wall -pedantic -g
INC_FLAGS = -I$(INC)
########################################

# Filename macroes
# server 
SERVER_OBJ = $(OBJ)tcp_router.o
UTIL_H = $(INC)tcp_util.h
UTIL_OBJ = $(OBJ)tcp_util.o
# all
ALL_OBJ = $(SERVER_OBJ) $(UTIL_OBJ) $(PROX_OBJ) $(UDP_SERVER_OBJ)
ALL_H = $(UTIL_H)
EXEC = $(SRV_DIR)tcp_router
########################################
# Recipes
.PHONY: server all clean

all: $(TARGET)

tcp_router : $(SERVER_OBJ) $(UTIL_OBJ)
	$(CC) $^ -o $(SRV_DIR)$@


$(SERVER_OBJ) : $(SRC)tcp_router.c $(UTIL_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@
# UTIL OBJ FILES
$(UTIL_OBJ) : $(SRC)tcp_util.c $(UTIL_H)
	$(CC) $(INC_FLAGS) $(C_FLAGS) -c $< -o $@

clean:
	rm -f $(ALL_OBJ)
	rm -f $(EXEC)
	rmdir $(OBJ)
	rmdir $(SRV_DIR)
