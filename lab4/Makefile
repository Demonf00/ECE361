CC=gcc
all:server client test
server: server.c
	$(CC) -o server server.c -lm
client: client.c
	$(CC) -o client client.c -lm
test: test.c
	$(CC) -o test test.c -lm
clean:
	rm -f *.o server client test
