CC=gcc
all:server deliver
server: server.o
deliver: deliver.c
	$(CC) -o deliver deliver.c -lm
clean:
	rm -f *.o server deliver
