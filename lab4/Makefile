CC=gcc
all:server client
server: server.c
	$(CC) -o server server.c -lm
client: client.c
	$(CC) -o client client.c -lm
clean:
	rm -f *.o server client
