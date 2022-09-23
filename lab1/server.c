#include<sys/types.h>//structure
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>//adress
#include<arpa/inet.h>//format convert
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Missing port number or too many arguments!\nThe correct usage is: server port\n");
	    exit(1);
    }
    int port = atoi(argv[1]);
    printf("hello world\n");
    return 0;
}