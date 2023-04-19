CC=gcc
CFLAGS=-Wall -c
SOURCEPATH=/source
all: client server cleanobj

client: client.o \
groupjoin.o \
groupleave.o \
groupmessage.o \
groups.o \
help.o \
login.o \
logout.o \
members.o \
online.o \
usermessage.o
	$(CC) $^ -o $@

server: server.o
	$(CC) $^ -o $@

%.o: source/%.c
	$(CC) $(CFLAGS) $< -o $@

cleanobj:
	rm *.o

clean:
	rm client server

