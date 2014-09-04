.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=server
OBJS=main.o sysutil.o session.o strutil.o ftp_nobody.o \
	 ftp_proto.o parse_conf.o configure.c priv_sock.o\
	 command_map.c trans_data.o priv_command.o
LIB=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(STARD)
clean:
	rm -f *.o $(BIN)
