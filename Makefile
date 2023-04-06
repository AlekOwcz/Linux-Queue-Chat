CC=gcc
CFLAGS=-Wall -c

all: client server cleanobj

client: inf148172_k.o inf148172_groupjoin.o inf148172_groupleave.o inf148172_groupmessage.o inf148172_groups.o inf148172_help.o inf148172_login.o inf148172_logout.o inf148172_members.o inf148172_online.o inf148172_usermessage.o
	$(CC) $^ -o $@

server: inf148172_s.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

cleanobj:
	rm *.o

clean:
	rm client server

