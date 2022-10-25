CC=gcc
all:server deliver
server: server.c
	$(CC) -o server server.c -lm
deliver: deliver.c
	$(CC) -o deliver deliver.c -lm
clean:
	rm -f *.o server deliver
