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
        fprintf(stderr, "Missing port number or too many arguments!\nThe correct usage should be: server port\n");
	    exit(1);
    }
    char IP[] = "192.168.91.128";
    int port = atoi(argv[1]);
    int client_socketfd;
    struct sockaddr_in remote_address;
    char buf[BUFSIZ];
    memset(&remote_address,0,sizeof(remote_address));

    remote_address.sin_family = AF_INET;//ip conversation channel
    remote_address.sin_addr.s_addr = inet_addr(IP);
    remote_address.sin_port = htons(port);

    printf("Set!\n");
    if((client_socketfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        fprintf(stderr, "Socket error!\n");
	    exit(1);
    }

    printf("Set!\n");
    if(connect(client_socketfd, (struct sockaddr*)&remote_address, sizeof(struct sockaddr)) < 0)
    {
        fprintf(stderr, "Connected error!\n");
	    exit(1);
    }

    printf("Set!\n");
    int len = recv(client_socketfd, buf, BUFSIZ, 0);
    buf[len] = '\0';
    printf("Port %d is open with protocol %s\n", port, buf);
    return 0;
}